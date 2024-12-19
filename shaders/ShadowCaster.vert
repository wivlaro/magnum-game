uniform highp mat4 transformationMatrix;
in highp vec4 position;

#ifdef ENABLE_MAX_ANIMATION_BONES
uniform uint perVertexJointCount;
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

void main()
{
	vec4 modelPosition = position;

	#ifdef ENABLE_MAX_ANIMATION_BONES
    if (perVertexJointCount > 0u) {
		modelPosition = getSkinMatrix() * modelPosition;
	}
	#endif

	gl_Position = transformationMatrix * modelPosition;
}
