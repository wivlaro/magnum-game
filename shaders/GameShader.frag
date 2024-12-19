uniform highp vec3 light;
uniform mediump vec3 ambientColor;
uniform mediump vec3 lightColor;
uniform mediump float shininess;

uniform highp sampler2D lightmapTexture;
uniform highp sampler2DArrayShadow shadowmapTexture;
uniform highp sampler2D diffuseTexture;
uniform mediump vec3 specularColor;

in mediump vec3 transformedNormal;
in highp vec3 cameraDirection;

in mediump vec2 interpolatedTextureCoords;

#ifdef ENABLE_SHADOWMAP_LEVELS
in highp vec3 shadowCoord[ENABLE_SHADOWMAP_LEVELS];
uniform highp float shadowDepthSplits[ENABLE_SHADOWMAP_LEVELS];
#endif

in highp vec3 worldPos;
uniform highp mat4 modelMatrix;

layout(location = 0) out lowp vec4 color;
layout(location = 1) out highp uint fragmentObjectId;


#ifdef ENABLE_SHADOWMAP_LEVELS
highp float computeShadowAtLevel(mediump vec3 levelShadowCoord, int shadowLevel, mediump vec3 normal, mediump vec3 lightDir) {
    highp float baseBias = 0.0010 + float(shadowLevel) * 0.001;
    highp float slopeScaledBias = baseBias * max(1.0 - dot(normal, lightDir), 0.0); // Slope-scaled bias
    highp float dZdx = dFdx(levelShadowCoord.z);
    highp float dZdy = dFdy(levelShadowCoord.z);
    highp float depthOffset = max(abs(dZdx), abs(dZdy)) * baseBias;

    highp float bias = depthOffset + slopeScaledBias;

    #ifdef SHADOWMAP_PCF
    mediump vec2 texelSize = vec2(1.0) / vec2(textureSize(shadowmapTexture, 0).xy);
    highp float shadow = 0.0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            mediump vec2 offset = vec2(x, y) * texelSize;
            shadow += texture(shadowmapTexture, vec4(levelShadowCoord.xy + offset, shadowLevel, clamp(levelShadowCoord.z - bias, 0.0, 1.0)));
        }
    }
    return shadow / 9.0; // Average PCF samples
    #else
//	return texture(shadowmapTexture, vec4(levelShadowCoord.xy, shadowLevel, clamp(levelShadowCoord.z - bias, 0.0, 1.0)));
    return texture(shadowmapTexture, vec4(levelShadowCoord.xy, shadowLevel, levelShadowCoord.z - bias));
    #endif
}
mediump float computeShadow(mediump vec3 normalizedTransformedNormal) {
    lowp float intensity = dot(normalizedTransformedNormal, light);
    if (intensity > 0.0) {
        for (int shadowLevel = 0; shadowLevel < ENABLE_SHADOWMAP_LEVELS; shadowLevel++) {
            mediump vec3 levelShadowCoord = shadowCoord[shadowLevel];
            bool inRange = levelShadowCoord.x > 0.0 && levelShadowCoord.y > 0.0 && levelShadowCoord.x < 1.0 && levelShadowCoord.y < 1.0 && levelShadowCoord.z > 0.0 && levelShadowCoord.z < 1.0;
            if (inRange) {
                return computeShadowAtLevel(levelShadowCoord, shadowLevel, normalizedTransformedNormal, light);
            }
        }
    }
    return 0.0;
}
#endif

void main() {
    lowp vec3 diffuseColor = texture(diffuseTexture, interpolatedTextureCoords).xyz;

    mediump vec3 ambient = ambientColor;

    mediump vec3 normalizedTransformedNormal = normalize(transformedNormal);

    //     color.rgb = ambient * diffuseColor * lightColor * intensity;
    ambient *= diffuseColor.rgb;

    mediump vec3 diffuse = max(dot(normalizedTransformedNormal, normalize(light)), 0.0) * diffuseColor.rgb * lightColor;

    mediump vec3 viewDir = normalize(cameraDirection); // Direction to the camera
    mediump vec3 reflectDir = reflect(-light, normalizedTransformedNormal);  // Reflect light around the normal
    mediump vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), shininess) * 0.5 * lightColor;

    mediump vec3 finalColor = ambient + diffuse + specular;

    #ifdef ENABLE_SHADOWMAP_LEVELS
    // Adjust shadow darkness as needed
    finalColor *= mix(0.5, 1.0, computeShadow(normalizedTransformedNormal));
    #endif

    fragmentObjectId = 0U;
    color = vec4(finalColor, 1.0); // Output final color with alpha

    //	color.rgb = vec3(0.5) + normalRaw * 0.5;
    //	color.rgb = diffuseColor;

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

