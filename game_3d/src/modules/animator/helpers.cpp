#include "helpers.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>

namespace game2d {

Bone
create_bone(const std::string& name, int id, const aiNodeAnim* channel)
{
  Bone b;
  b.name = name;
  b.id = id;

  for (int i = 0; i < channel->mNumPositionKeys; i++) {
    const auto aiPosition = channel->mPositionKeys[i].mValue;
    const auto timestamp = channel->mPositionKeys[i].mTime;
    KeyPosition pos;
    pos.position.x = aiPosition.x;
    pos.position.y = aiPosition.y;
    pos.position.z = aiPosition.z;
    pos.timestamp = timestamp;
    b.positions.push_back(pos);
  }

  for (int i = 0; i < channel->mNumRotationKeys; i++) {
    const auto aiOrientation = channel->mRotationKeys[i].mValue;
    const auto timestamp = channel->mRotationKeys[i].mTime;
    KeyRotation data;
    data.orientation.x = aiOrientation.x;
    data.orientation.y = aiOrientation.y;
    data.orientation.z = aiOrientation.z;
    data.orientation.w = aiOrientation.w;
    data.timestamp = timestamp;
    b.rotations.push_back(data);
  }

  for (int i = 0; i < channel->mNumScalingKeys; i++) {
    const auto scale = channel->mScalingKeys[i].mValue;
    const auto timestamp = channel->mScalingKeys[i].mTime;
    KeyScale data;
    data.scale.x = scale.x;
    data.scale.y = scale.y;
    data.scale.z = scale.z;
    data.timestamp = timestamp;
    b.scales.push_back(data);
  }

  return b;
};

int
get_position_index(const Bone& b, float time)
{
  for (int i = 0; i < b.positions.size() - 1; ++i)
    if (time < b.positions[i + 1].timestamp)
      return i;
  return 0;
};

int
get_rotation_index(const Bone& b, float time)
{
  for (int i = 0; i < b.rotations.size() - 1; ++i)
    if (time < b.rotations[i + 1].timestamp)
      return i;
  return 0;
};

int
get_scale_index(const Bone& b, float time)
{
  for (int i = 0; i < b.scales.size() - 1; ++i)
    if (time < b.scales[i + 1].timestamp)
      return i;
  return 0;
};

// Gets normalized value for Lerp & Slerp
float
get_scale_factor(float last, float next, float time)
{
  float scaleFactor = 0.0f;
  float midWayLength = time - last;
  float framesDiff = next - last;
  scaleFactor = midWayLength / framesDiff;
  return scaleFactor;
};

/*figures out which position keys to interpolate b/w and performs the interpolation
and returns the translation matrix*/
glm::mat4
interpolate_position(const Bone& b, float time)
{
  if (b.positions.size() == 1)
    return glm::translate(glm::mat4(1.0f), b.positions[0].position);

  const int p0 = get_position_index(b, time);
  const int p1 = p0 + 1;
  const auto& b0 = b.positions[p0];
  const auto& b1 = b.positions[p1];
  const float t = get_scale_factor(b0.timestamp, b1.timestamp, time);
  const glm::vec3 pos = glm::mix(b0.position, b1.position, t);
  return glm::translate(glm::mat4(1.0f), pos);
};

glm::mat4
interpolate_rotation(const Bone& b, float time)
{
  if (b.rotations.size() == 1) {
    const auto rotation = glm::normalize(b.rotations[0].orientation);
    return glm::toMat4(rotation);
  }

  const int p0 = get_rotation_index(b, time);
  const int p1 = p0 + 1;
  const auto& b0 = b.rotations[p0];
  const auto& b1 = b.rotations[p1];
  const float t = get_scale_factor(b0.timestamp, b1.timestamp, time);
  auto rot = glm::slerp(b0.orientation, b1.orientation, t);
  rot = glm::normalize(rot);
  return glm::toMat4(rot);
};

glm::mat4
interpolate_scale(const Bone& b, float time)
{
  if (b.scales.size() == 1)
    return glm::scale(glm::mat4(1.0f), b.scales[0].scale);

  const int p0 = get_scale_index(b, time);
  const int p1 = p0 + 1;
  const auto& b0 = b.scales[p0];
  const auto& b1 = b.scales[p1];
  const auto t = get_scale_factor(b0.timestamp, b1.timestamp, time);
  const auto scale = glm::mix(b0.scale, b1.scale, t);
  return glm::scale(glm::mat4(1.0f), scale);
};

void
update_bone(Bone& b, float time)
{
  const glm::mat4 translation = interpolate_position(b, time);
  const glm::mat4 rotation = interpolate_rotation(b, time);
  const glm::mat4 scale = interpolate_scale(b, time);
  b.local_transform = translation * rotation;
};

void
read_hierachy_data(AssimpNodeData& dest, const aiNode* src)
{
  const auto name = src->mName.C_Str();
  dest.name = name;
  // std::cout << "found: " << name << std::endl;

  // convert from aiMatrix4x4 to glm::mat4
  // clang-format off
  auto& from = src->mTransformation;
  auto& to = dest.transformation;
  //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
  to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
  to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
  to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
  to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
  // clang-format on

  for (int i = 0; i < src->mNumChildren; i++) {
    AssimpNodeData new_data;
    read_hierachy_data(new_data, src->mChildren[i]);
    dest.children.push_back(new_data);
  }
};

void
read_missing_bones(Animation& a, const aiAnimation* anim, const Model& m)
{
  const int size = anim->mNumChannels;

  const auto& bone_info = m.bone_info; // get bone info from model class
  const auto bone_count = m.bone_info.size();
  std::cout << "When loading animation, model has " << bone_count << " bones" << std::endl;

  for (int i = 0; i < size; i++) {
    const auto channel = anim->mChannels[i];
    const std::string name = channel->mNodeName.data;

    const auto bone =
      std::find_if(bone_info.begin(), bone_info.end(), [&name](const BoneInfo& b) { return b.name == name; });

    if (bone == bone_info.end()) {
      std::cerr << "(WARNING) Bone:" << name << "missing from model?" << std::endl;
      continue;
    }

    a.bones.push_back(create_bone(name, (*bone).id, channel));
  }

  a.bone_info = m.bone_info;
};

Animation
load_animation(Model& model)
{
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(model.path, aiProcess_Triangulate);
  assert(scene && scene->mRootNode);

  Animation data;

  // if (scene->mNumAnimations == 0)
  //   return data; // exit

  auto anim = scene->mAnimations[0];
  data.duration = anim->mDuration;
  data.ticks = anim->mTicksPerSecond;

  AssimpNodeData root_node;
  read_hierachy_data(root_node, scene->mRootNode);
  data.root = root_node;

  read_missing_bones(data, anim, model);

  // check animation bones & check model bones
  assert(model.bone_info.size() == data.bone_info.size());

  return data;
}

void
load_animations(SINGLE_AnimatorComponent& anims, SINGLE_ModelsComponent& models)
{
  // anims.animation_0_data = load_animation(models.low_poly_car);

  anims.final_bone_matrices.reserve(100);
  for (int i = 0; i < 100; i++)
    anims.final_bone_matrices.push_back(glm::mat4(1.0f));
}

//
// animator
//

void
play_animation(SINGLE_AnimatorComponent& anims, Animation* a)
{
  std::cout << "playing new animation" << std::endl;
  anims.current_animation = a;
  anims.current_time = 0.0f;

  anims.final_bone_matrices.clear();
  for (int i = 0; i < 100; i++)
    anims.final_bone_matrices.push_back(glm::mat4(1.0f));
}

void
calculate_bone_transforms(SINGLE_AnimatorComponent& anims, const AssimpNodeData* node, const glm::mat4& parent)
{
  const auto& name = node->name;
  auto transform = node->transformation;
  const auto bone = std::find_if(anims.current_animation->bones.begin(),
                                 anims.current_animation->bones.end(),
                                 [&name](const Bone& b) { return b.name == name; });
  if (bone != anims.current_animation->bones.end()) {
    update_bone((*bone), anims.current_time);
    transform = bone->local_transform;
  }

  const auto global_transformation = parent * transform;

  const auto& bone_info = anims.current_animation->bone_info;
  const auto has_bone_info =
    std::find_if(bone_info.begin(), bone_info.end(), [&name](const BoneInfo& bi) { return bi.name == name; });
  if (has_bone_info != bone_info.end()) {
    const auto index = (*has_bone_info).id;
    const auto offset = (*has_bone_info).offset;
    anims.final_bone_matrices[index] = global_transformation * offset;
  }

  for (int i = 0; i < node->children.size(); i++)
    calculate_bone_transforms(anims, &node->children[i], global_transformation);
}

void
update_animation(SINGLE_AnimatorComponent& anims, float dt)
{
  if (anims.current_animation) {
    anims.current_time += anims.current_animation->ticks * dt;
    anims.current_time = fmod(anims.current_time, anims.current_animation->duration);

    std::cout << "animation current time: " << anims.current_time << std::endl;

    calculate_bone_transforms(anims, &anims.current_animation->root, glm::mat4(1.0f));
  }
}

} // namespace game2d