#version 330 core
layout (location = 0) in vec3  aPos;
layout (location = 1) in vec3  aNormal;
layout (location = 2) in vec2  aTexCoords;
layout (location = 3) in vec3  tangent;
layout (location = 4) in vec3  bitangent;
layout (location = 5) in vec4 bIDs1;     // N max bones per vertex
layout (location = 6) in vec4 bIDs2;     // N max bones per vertex
layout (location = 7) in vec4 bIDs3;     // N max bones per vertex
layout (location = 8) in vec4  bWeights1;   // N max bones per vertex
layout (location = 9) in vec4  bWeights2;   // N max bones per vertex
layout (location = 10) in vec4  bWeights3;   // N max bones per vertex


out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

//out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 gBones[100];


void main()
{

    vs_out.FragPos = vec3(model * vec4(aPos, 1.0)); 
	mat3 normalMatrix = transpose(inverse(mat3(model)));
    vs_out.Normal = normalize(normalMatrix * aNormal);	
	
    mat4 BoneTransform = gBones[int(bIDs1[0])] * bWeights1[0];
    BoneTransform += gBones[int(bIDs1[1])] * bWeights1[1];
    BoneTransform += gBones[int(bIDs1[2])] * bWeights1[2];
    BoneTransform += gBones[int(bIDs1[3])] * bWeights1[3];  // only take the first 4th bones contributions

    BoneTransform += gBones[int(bIDs2[0])] * bWeights2[0];
    BoneTransform += gBones[int(bIDs2[1])] * bWeights2[1];
    BoneTransform += gBones[int(bIDs2[2])] * bWeights2[2];
    BoneTransform += gBones[int(bIDs2[3])] * bWeights2[3];  // only take the next bones contributions

    BoneTransform += gBones[int(bIDs3[0])] * bWeights3[0];
    BoneTransform += gBones[int(bIDs3[1])] * bWeights3[1];
    BoneTransform += gBones[int(bIDs3[2])] * bWeights3[2];
    BoneTransform += gBones[int(bIDs3[3])] * bWeights3[3]; // only take the next bones contributions

    vec4 PosL = BoneTransform * vec4(aPos, 1.0f);
    gl_Position = projection * view * model * PosL;

    vs_out.TexCoords = aTexCoords;    
    //gl_Position = projection * view * model * vec4(aPos, 1.0);
}