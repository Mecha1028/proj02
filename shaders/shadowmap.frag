// shadowmap.frag
#version 410

in vec3 fragPos;
in vec3 normal;
in vec2 texCoord;

// LabA09 ShadowMap TODO: add input variable fragPosLightSpace


uniform bool hasTexture;
uniform sampler2D textureMap;

// LabA09 ShadowMap TODO: add uniform texture shadowMap


uniform vec3 lightPos;
uniform vec3 viewPos;

out vec4 colour_out;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // LabA09 ShadowMap TODO:
    // calculate the normalised coordinate of the projected fragment in light space
    // and convert the coordinate from [-1, 1] to [0, 1]
    vec3 projCoords = /* TODO */;
    
    // LabA09 ShadowMap TODO:
    // use x and y of the converted coorindate to read closeDepth from the depth map



    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    
    float shadow = 0.0;
    
    // LabA09 ShadowMap TODO:
    // compare currentDepth with closeDepth
    // if in shadow, set shadow to 1.0

 
    
    return shadow;
}

void main()
{   
    vec3 colour = vec3(1.0, 0.0, 0.0);
    if (hasTexture)
        colour = texture(textureMap, texCoord).rgb;
    

    // 1. ambient
    vec3 ambient = 0.05 * colour;

    // 2. diffuse
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 norm = normalize(normal);
    float diff = max(dot(lightDir, norm), 0.0);
    vec3 diffuse = diff * colour;
    
    // 3. specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    // the shininess coefficient beta = 32.0 
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);

    // assuming a light source with a bright white colour
    vec3 specular = vec3(0.3) * spec;   
    
    // calculate shadow using the light space position
    float shadow = ShadowCalculation(fragPosLightSpace);

    // only render ambient colour if in shadow
    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);    
    
    colour_out = vec4(lighting, 1.0);
}