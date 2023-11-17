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
  b.local_transform = translation * rotation * scale;
};

void
read_hierachy_data(AssimpNodeData& dest, const aiNode* src)
{
  const auto name = src->mName.C_Str();
  std::cout << "found: " << name << std::endl;

  const auto transformation = src->mTransformation;
  for (int i = 0; i < src->mNumChildren; i++) {
    AssimpNodeData new_data;
    read_hierachy_data(new_data, src->mChildren[i]);
    dest.children.push_back(new_data);
  }
};

void
read_missing_bones(Animation& a, const aiAnimation* anim, Model& m)
{
  const int size = anim->mNumChannels;

  // Get info from Model class
  auto& bone_info = m.bone_info;
  auto bone_count = m.bone_info.size();

  for (int i = 0; i < size; i++) {
    const auto channel = anim->mChannels[i];
    const std::string name = channel->mNodeName.data;

    const auto bone =
      std::find_if(bone_info.begin(), bone_info.end(), [&name](const BoneInfo& b) { return b.name == name; });

    if (bone == bone_info.end()) {
      // missin bone info?
      continue;
    }

    BoneInfo new_bone;
    create_bone(name, (*bone).id, channel);
    bone_info.push_back(new_bone);
  }

  a.bone_info = bone_info;
};

Animation
load_animation(Model& model)
{
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(model.path, aiProcess_Triangulate);

  Animation data;
  auto anim = scene->mAnimations[0];
  data.duration = anim->mDuration;
  data.ticks = anim->mTicksPerSecond;

  AssimpNodeData root_node;
  aiNode* root = scene->mRootNode;
  read_hierachy_data(root_node, root);
  read_missing_bones(data, anim, model);

  return data;
}

void
load_animations(SINGLE_AnimatorComponent& anims, SINGLE_ModelsComponent& models)
{
  anims.animation_0_data = load_animation(models.low_poly_car);

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
}

void
calculate_bone_transforms(SINGLE_AnimatorComponent& anims,
                          const AssimpNodeData* node,
                          const glm::mat4& parent,
                          const float dt)
{
  const auto& name = node->name;
  auto transform = node->transformation;

  const auto bone = std::find_if(anims.current_animation->bones.begin(),
                                 anims.current_animation->bones.end(),
                                 [&name](const Bone& b) { return b.name == name; });

  if (bone != anims.current_animation->bones.end()) {
    update_bone((*bone), dt);
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
    calculate_bone_transforms(anims, &node->children[i], global_transformation, dt);
}

void
update_animation(SINGLE_AnimatorComponent& anims, float dt)
{
  if (anims.current_animation) {
    anims.current_time += anims.current_animation->ticks * dt;
    anims.current_time = fmod(anims.current_time, anims.current_animation->duration);
    calculate_bone_transforms(anims, &anims.current_animation->root, glm::mat4(1.0f), dt);
  }
}

} // namespace game2d