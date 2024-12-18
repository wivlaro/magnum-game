uniform highp vec3 light;
uniform vec3 ambientColor = vec3(0.25, 0.25, 0.25);
uniform vec3 lightColor = vec3(1.0, 1.0, 1.0);
uniform float shininess = 80.0;

uniform sampler2D lightmapTexture;
uniform sampler2DArrayShadow shadowmapTexture;
uniform sampler2D diffuseTexture;
uniform vec3 specularColor = vec3(1.0, 1.0, 1.0);

in mediump vec3 transformedNormal;
in highp vec3 cameraDirection;

in mediump vec2 interpolatedTextureCoords;
in highp vec3 shadowCoord[ENABLE_SHADOWMAP_LEVELS];
uniform float shadowDepthSplits[ENABLE_SHADOWMAP_LEVELS];

out lowp vec4 color;

in vec3 worldPos;

uniform mat4 modelMatrix;

in highp vec3 normalRaw;



float computeShadow(vec3 levelShadowCoord, int shadowLevel, vec3 normal, vec3 lightDir) {
    float baseBias = 0.0010 + shadowLevel * 0.001;
    float slopeScaledBias = baseBias * max(1.0 - dot(normal, lightDir), 0.0); // Slope-scaled bias
    float dZdx = dFdx(levelShadowCoord.z);
    float dZdy = dFdy(levelShadowCoord.z);
    float depthOffset = max(abs(dZdx), abs(dZdy)) * baseBias;

    float bias = depthOffset + slopeScaledBias;

    #ifdef SHADOWMAP_PCF
	{
        vec2 texelSize = vec2(1.0) / textureSize(shadowmapTexture, 0).xy;
        float shadow = 0.0;
        for (int x = -1; x <= 1; ++x) {
            for (int y = -1; y <= 1; ++y) {
                vec2 offset = vec2(x, y) * texelSize;
                shadow += texture(shadowmapTexture, vec4(levelShadowCoord.xy + offset, shadowLevel, clamp(levelShadowCoord.z - bias, 0.0, 1.0)));
            }
        }
        return shadow / 9.0; // Average PCF samples
    }
    #else
//	return texture(shadowmapTexture, vec4(levelShadowCoord.xy, shadowLevel, clamp(levelShadowCoord.z - bias, 0.0, 1.0)));
    return texture(shadowmapTexture, vec4(levelShadowCoord.xy, shadowLevel, levelShadowCoord.z - bias));
    #endif
}

void main() {
    lowp vec3 diffuseColor = texture(diffuseTexture, interpolatedTextureCoords).xyz;

/* Ambient color */
    vec3 ambient = ambientColor;

    mediump vec3 normalizedTransformedNormal = normalize(transformedNormal);

    float shadow = 0.0;
    lowp float intensity = dot(normalizedTransformedNormal, light);
    int shadowLevel = 0;
    if (intensity <= 0) {
        shadow = 0.0f;
        intensity = 0.0f;
    }
    else {
        for (; shadowLevel < ENABLE_SHADOWMAP_LEVELS; shadowLevel++) {
            vec3 levelShadowCoord = shadowCoord[shadowLevel];
            bool inRange = levelShadowCoord.x > 0 && levelShadowCoord.y > 0 && levelShadowCoord.x < 1 && levelShadowCoord.y < 1 && levelShadowCoord.z > 0 && levelShadowCoord.z < 1;
            if (inRange) {
                float bias = 0.0015;
                //				float bias = 0.0015 * max(1.0 - dot(normalizedTransformedNormal, light), 0.0);
                shadow = computeShadow(levelShadowCoord, shadowLevel, normalizedTransformedNormal, light);
                //				shadow = texture(shadowmapTexture, vec4(levelShadowCoord.xy, shadowLevel, levelShadowCoord.z-bias));
                break;
            }
        }
    }
    //     color.rgb = ambient * diffuseColor * lightColor * intensity;
    ambient *= diffuseColor.rgb;

    vec3 diffuse = max(dot(normalizedTransformedNormal, normalize(light)), 0.0) * diffuseColor.rgb * lightColor;

    vec3 viewDir = normalize(cameraDirection); // Direction to the camera
    vec3 reflectDir = reflect(-light, normalizedTransformedNormal);  // Reflect light around the normal
    //	vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), shininess) * 0.5 * lightColor;
    vec3 specular = vec3(0);
    vec3 shadowFactor = mix(vec3(0.5), vec3(1.0), shadow); // Adjust shadow darkness as needed

    vec3 finalColor = shadowFactor * (ambient + diffuse + specular);
//    finalColor = vec3(shadowFactor.x, 0.5, 0.5);

    color = vec4(finalColor, 1.0); // Output final color with alpha

    //	color.rgb = vec3(0.5) + normalRaw * 0.5;
    //	color.rgb = diffuseColor;

    //	color.rgb = vec3(intensity);
    //	color.rgb = vec3(interpolatedTextureCoords, 0.0);
    //    color.rgb = ((0.1 + vec3(intensity * shadow)) * diffuseColor);

    //	color.rgb = diffuseColor * (inRange ? clamp(shadowCoord[shadowLevel],0,1) : vec3(0.1,0.1,0.1));
    //	vec4 shadowTexel = texture(shadowmapTexture, vec3(shadowCoord[shadowLevel].xy, shadowLevel));
    //	shadowTexel.g = (1+shadowLevel) / 6.0;
    //	shadowTexel.b = shadowCoord[shadowLevel].z;
    //	color.rgb = diffuseColor * shadowTexel.rgb;
    //	color.rgb = diffuseColor * (shadow + 1);

    //    if (shadowLevel == 0) {
    //    	color.rgb *= vec3(1,0,0);
    //    }
    //    else if (shadowLevel == 1) {
    //    	color.rgb *= vec3(1,1,0);
    //    }
    //    else if (shadowLevel == 2) {
    //    	color.rgb *= vec3(0,1,0);
    //    }
    //    else if (shadowLevel == 3) {
    //    	color.rgb *= vec3(0,1,1);
    //    }
    //    else if (shadowLevel == 4) {
    //    	color.rgb *= vec3(0,0,1);
    //    }
    //    else {
    //    	color.rgb *= vec3(1,0,1);
    //    }


    //     if(intensity > 0.001) {
    //         highp vec3 reflection = reflect(-normalizedLightDirection, normalizedTransformedNormal);
    //         mediump float specularity = pow(max(0.0, dot(normalize(cameraDirection), reflection)), shininess);
    //         color.rgb += specularColor*specularity;
    //     }

    //    color.a = 1.0;
}

