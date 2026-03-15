#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
const float offset = 1.0 / 300.0;  
uniform int postfx;

void main()
{
    //Deslocamento 
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right     
        );
    // Color 
    vec3 color = texture(screenTexture, TexCoords).rgb;

    switch(postfx)
    {
    case 1: // Default State
        color = texture(screenTexture, TexCoords).rgb;
        FragColor = vec4(color, 1.0);
        break;

    case 2: //invert color
        color = texture(screenTexture, TexCoords).rgb;
        vec3 invert = 1.0 - color;
        FragColor = vec4(invert, 1.0);
        break;

    case 3: //Grayscale
        color = texture(screenTexture, TexCoords).rgb;
        float average = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
        FragColor = vec4(vec3(average), 1.0);
        break;

    case 4://Toxic
       float kernelToxic[9] = float[](
            -1, -1, -1,
            -1,  9, -1,
            -1, -1, -1
        );
        vec3 toxicSample[9];
        for(int i = 0; i < 9; i++)
            {
            toxicSample[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
            }
        color = vec3(0.0);
        for(int i = 0; i < 9; i++)
            color += toxicSample[i] * kernelToxic[i];
        FragColor = vec4(color, 1.0);
       break;

    case 5:// Blur
        float kernelBlur[9] = float[](
            1.0 / 16, 2.0 / 16, 1.0 / 16,
            2.0 / 16, 4.0 / 16, 2.0/ 16,
            1.0 / 16, 2.0 / 16, 1.0 / 16  
        );
        vec3 blurSample[9];
        for(int i = 0; i < 9; i++)
            {
                blurSample[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
            }
        color = vec3(0.0);
        for(int i = 0; i < 9; i++)
            color += blurSample[i] * kernelBlur[i];
        FragColor = vec4(color, 1.0);
        break;

    case 6:// Edge detect
        float kernelEdge[9] = float[](
            1, 1, 1,
            1, -8, 1,
            1, 1, 1
        );
        vec3 edgeSample[9];
        for(int i = 0; i < 9; i++)
            {
                edgeSample[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
            }
        color = vec3(0.0);
        for(int i = 0; i < 9; i++)
            color += edgeSample[i] * kernelEdge[i];
        FragColor = vec4(color, 1.0);
        break;
    default: // Default State
        color = texture(screenTexture, TexCoords).rgb;
        FragColor = vec4(color, 1.0);
        break;
    }
} 