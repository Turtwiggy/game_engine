
// //header
// #include "engine/3d/model.hpp"

// //your project files
// #include "engine/graphics/texture.hpp"
// #include "engine/graphics/shader.hpp"
// #include "engine/graphics/vertex.hpp"
// #include "engine/resources/resource_manager.hpp"

// namespace fightingengine {

// // std::vector<FETriangle> Model::get_all_triangles_in_meshes()
// // {
// //     std::vector<FETriangle> tris;
// //     for (auto &m : meshes)
// //     {
// //         for (auto &t : m.triangles)
// //         {
// //             tris.push_back(t);
// //         }
// //     }
// //     //printf("model: %s has %i triangles", name, tris.size());
// //     return tris;
// // }


// void Model::init(const aiScene* raw_model, const std::string& name)
// {
//     process_node(raw_model->mRootNode, raw_model);
// }

// std::vector<Mesh>& Model::get_meshes()
// {
//     return meshes;
// }



// void Model::process_node(aiNode *node, const aiScene *scene)
// {
//     // process all the node's meshes (if any)
//     for (unsigned int i = 0; i < node->mNumMeshes; i++)
//     {
//         aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
//         meshes.push_back(process_mesh(mesh, scene));
//     }
//     // then do the same for each of its children
//     for (unsigned int i = 0; i < node->mNumChildren; i++)
//     {
//         process_node(node->mChildren[i], scene);
//     }
// }

// Mesh Model::process_mesh(aiMesh *mesh, const aiScene *scene)
// {
//     // data to fill
//     std::vector<Vertex> vertices;
//     std::vector<unsigned int> indices;
//     std::vector<Texture2D> textures;

//     // walk through each of the mesh's vertices
//     for (unsigned int i = 0; i < mesh->mNumVertices; i++)
//     {
//         Vertex vertex;
//         glm::vec3 vector;
        
//         // positions
//         vector.x = mesh->mVertices[i].x;
//         vector.y = mesh->mVertices[i].y;
//         vector.z = mesh->mVertices[i].z;
//         vertex.Position = vector;

//         // normals
//         if (mesh->HasNormals())
//         {
//             vector.x = mesh->mNormals[i].x;
//             vector.y = mesh->mNormals[i].y;
//             vector.z = mesh->mNormals[i].z;
//             vertex.Normal = vector;
//         }

//         // texture coordinates
//         if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
//         {
//             glm::vec2 vec;
//             // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
//             // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
//             vec.x = mesh->mTextureCoords[0][i].x;
//             vec.y = mesh->mTextureCoords[0][i].y;
//             vertex.TexCoords = vec;
//         }
//         else
//             vertex.TexCoords = glm::vec2(0.0f, 0.0f);

//         //if (mesh->HasTangentsAndBitangents())
//         //{
//         //    // tangent
//         //    vector.x = mesh->mTangents[i].x;
//         //    vector.y = mesh->mTangents[i].y;
//         //    vector.z = mesh->mTangents[i].z;
//         //    vertex.Tangent = vector;
//         //    // bitangent
//         //    vector.x = mesh->mBitangents[i].x;
//         //    vector.y = mesh->mBitangents[i].y;
//         //    vector.z = mesh->mBitangents[i].z;
//         //    vertex.Bitangent = vector;
//         //}
//         //else
//         //{
//         //    //printf("-> %s has no tangent and bittangents. \n", mesh->mName.C_Str());
//         //}

//         vertices.push_back(vertex);
//     }

//     // now wak through each of the mesh's faces 
//     // and retrieve the corresponding vertex indices.
//     for (unsigned int i = 0; i < mesh->mNumFaces; i++)
//     {
//         aiFace face = mesh->mFaces[i];
//         // retrieve all indices of the face and store them in the indices vector
//         for (unsigned int j = 0; j < face.mNumIndices; j++)
//             indices.push_back(face.mIndices[j]);
//     }

//     // process materials
//     aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
//     // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
//     // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
//     // Same applies to other texture as the following list summarizes:
//     // diffuse: texture_diffuseN
//     // specular: texture_specularN
//     // normal: texture_normalN

//     // // 1. diffuse maps
//     // std::vector<Texture2D> diffuseMaps = load_material_textures(material, aiTextureType_DIFFUSE, "texture_diffuse");
//     // textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
//     // // 2. specular maps
//     // std::vector<Texture2D> specularMaps = load_material_textures(material, aiTextureType_SPECULAR, "texture_specular");
//     // textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
//     // // 3. normal maps
//     // std::vector<Texture2D> normalMaps = load_material_textures(material, aiTextureType_HEIGHT, "texture_normal");
//     // textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
//     // // 4. height maps
//     // std::vector<Texture2D> heightMaps = load_material_textures(material, aiTextureType_AMBIENT, "texture_height");
//     // textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

//     //Give each mesh its colour
//     aiColor3D mat_colour(1.f, 1.f, 1.f);
//     material->Get(AI_MATKEY_COLOR_DIFFUSE, mat_colour);

//     ColourVec4f colour(glm::vec4(mat_colour.r, mat_colour.g, mat_colour.b, 1.0f));

//     //calculate normals if no normals and store info in vetex array
//     //if (!mesh->HasNormals())
//     //{
//     //    printf("mesh had no normals - calculating");
//     //    //Newell's method
//     //    //https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal#:~:text=A%20surface%20normal%20for%20a,of%20the%20face%20w.r.t.%20winding).
//     //    int num_verts = vertices.size();
//     //    for (int i = 0; i < num_verts; i++)
//     //    {
//     //        FGVertex& cur = vertices[i];
//     //        FGVertex& nxt = vertices[(i+1) % num_verts];
//     //        //cur.Normal.x += (cur.Position.y - nxt.Position.y) * (cur.Position.z + nxt.Position.z);
//     //        //cur.Normal.y += (cur.Position.z - nxt.Position.z) * (cur.Position.x + nxt.Position.x);
//     //        //cur.Normal.z += (cur.Position.x - nxt.Position.x) * (cur.Position.y + nxt.Position.y);
//     //        vertices[i].Normal += glm::cross(cur.Position, nxt.Position);
//     //    }
//     //}

//     // return a mesh object created from the extracted mesh data
//     return Mesh(material->GetName().C_Str(), vertices, indices, textures, colour );
// }

// // // checks all material textures of a given type and loads the textures if they're not loaded yet. the required info is returned as a Texture struct.
// // std::vector<Texture2D> Model::load_material_textures(aiMaterial *mat, aiTextureType type, std::string typeName)
// // {
// //     std::vector<Texture2D> textures;
// //     for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
// //     {
// //         aiString str;
// //         mat->GetTexture(type, i, &str);
// //         std::string full_path = directory + '/' + str.C_Str();

// //         //check cached textures
// //         for (unsigned int j = 0; j < textures_loaded.size(); j++)
// //         {
// //             if (textures_loaded[j].path.data() != full_path)
// //             {
// //                 textures.push_back(textures_loaded[j]);
// //                 continue;
// //             }
// //         }

// //         printf("Texture not loaded, path: %s name: %s \n", full_path.c_str(), str.C_Str());
// //         Texture2D texture = ResourceManager::load_texture(full_path.c_str(), false, str.C_Str());
// //         texture.type = typeName;
// //         texture.path = full_path;
// //         textures.push_back(texture);

// //         textures_loaded.push_back(texture); // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
// //     }

// //     return textures;
// // }

// } // namespace fightingengine
