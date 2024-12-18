
uniform highp mat4 transformationMatrix;
uniform highp mat4 projectionMatrix;
uniform mediump mat3 normalMatrix;
uniform mat4 modelMatrix;

layout(location = 0) in highp vec3 position;
layout(location = 1) in mediump vec2 textureCoordinates;
layout(location = 2) in highp vec3 normal;

out mediump vec2 interpolatedTextureCoords;
out mediump vec3 transformedNormal;
out highp vec3 lightDirection;
out highp vec3 cameraDirection;

#ifdef ENABLE_SHADOWMAP_LEVELS
uniform highp mat4 shadowmapMatrix[ENABLE_SHADOWMAP_LEVELS];
out highp vec3 shadowCoord[ENABLE_SHADOWMAP_LEVELS];
#endif

#ifdef ENABLE_MAX_ANIMATION_BONES
uniform int perVertexJointCount;
uniform mat4 jointMatrices[ENABLE_MAX_ANIMATION_BONES];

layout(location = 6) in mediump uvec4 jointIds;
layout(location = 7) in mediump vec4 weights;
mat4 getSkinMatrix() {
    mat4 skinMatrix = mat4(0.0);
    for(uint i = 0u; i != perVertexJointCount; ++i) {
        skinMatrix += weights[i]*jointMatrices[jointIds[i]];
    }
    return skinMatrix;
}
#endif

void main() {
    vec4 position4 = vec4(position, 1.0);

    vec3 modelNormal = normal;
    #ifdef ENABLE_MAX_ANIMATION_BONES
    if (perVertexJointCount > 0) {
        mat4 skinMatrix = getSkinMatrix();
        position4 = skinMatrix * position4;
        modelNormal = mat3(skinMatrix) * normal;
    }
    #endif

    highp vec4 transformedPosition4 = transformationMatrix*position4;
    highp vec3 transformedPosition = transformedPosition4.xyz/transformedPosition4.w;

    vec4 worldPos4 = modelMatrix * position4;

    transformedNormal = normalMatrix*modelNormal;

    cameraDirection = -transformedPosition;

    for (int i = 0; i < shadowmapMatrix.length(); i++) {
        //        float castReceiveOffsetBias = 0.10 + i * 0.001;
        //        shadowCoord[i] = (shadowmapMatrix[i] * (worldPos4 + vec4(transformedNormal * castReceiveOffsetBias,0))).xyz;
        shadowCoord[i] = (shadowmapMatrix[i] * worldPos4).xyz;
    }

    gl_Position = projectionMatrix*transformedPosition4;

    interpolatedTextureCoords = textureCoordinates;
}
