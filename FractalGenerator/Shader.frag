#version 330 core
in vec4 gl_FragCoord;
 
out vec4 fragColor;
out float gl_FragDepth;

uniform vec4 color_0;
uniform vec4 color_1;
uniform vec4 color_2;
uniform vec4 color_3;
 
uniform float center_x;
uniform float center_y;
uniform float julia_real;
uniform float julia_imag;
uniform float zoom;
uniform float linear_zoom;
uniform vec4 color_ranges;
uniform float fractalType;

uniform float sierpinski_xFactor;
uniform float sierpinski_yFactor;
uniform float sierpinski_xLower;
uniform float sierpinski_xUpper;
uniform float sierpinski_yLower;
uniform float sierpinski_yUpper;
 
#define MAX_ITERATIONS 600
 
int get_iterations_julia()
{
    float real = ((gl_FragCoord.x / 1080.0f - 0.5f) * zoom + center_x) * 4.0f;
    float imag = ((gl_FragCoord.y / 1080.0f - 0.5f) * zoom + center_y) * 4.0f;

    int iterations = 0;

    while (iterations < MAX_ITERATIONS)
    {
        float tmp_real = real;
        real = (real * real - imag * imag) + julia_real;
        imag = (2.0 * tmp_real * imag) + julia_imag;

        float dist = real * real + imag * imag;

        if (dist > 4.0)
            break;

        ++iterations;
    }
    return iterations;
}

int get_iterations_mandelbrot()
{
    float real = ((gl_FragCoord.x / 1080.0f - 0.5f) * zoom + center_x) * 4.0f;
    float imag = ((gl_FragCoord.y / 1080.0f - 0.5f) * zoom + center_y) * 4.0f;
 
    int iterations = 0;
    float const_real = real;
    float const_imag = imag;
 
    while (iterations < MAX_ITERATIONS)
    {
        float tmp_real = real;
        real = (real * real - imag * imag) + const_real;
        imag = (2.0 * tmp_real * imag) + const_imag;
         
        float dist = real * real + imag * imag;
 
        if (dist > 4.0)
            break;
 
        ++iterations;
    }
    return iterations;
}

vec4 sierpinski()
{
    vec2 fragCoord = gl_FragCoord.xy;
    vec2 iResolution = vec2(1080.0f, 1080.0f);
    vec2 pos = fragCoord - iResolution.xy / 2.0;
    vec2 offset = vec2(-0.5);
    
    float col = 0.0;
    float t = linear_zoom;
    
    float scale = pow(3.0, mod(t, 2.0) + 1.0);
    float size = iResolution.y * scale;
    float rot = 0;

    pos = mat2(cos(-rot), sin(-rot), -sin(-rot), cos(-rot)) * pos;
    pos += offset * iResolution.y * (scale * 0.5 - 0.5);

    while(size > 1.0) {
        size /= 3.0;
        ivec2 ip = ivec2(round(pos / size));

        if(ip.x == 0 && ip.y == 0) {
            col = min(size*size, 1.0);
            break;
        } else {
            pos -= vec2(ip) * size;
        }
    }

    if(col == 0.0) {
        fragColor = color_0;
    }else{
        fragColor = color_1;
    }
    return fragColor;
}

 
vec4 return_color()
{
    int iter = 0;  // Declare iter outside the if blocks

    if (fractalType == 0.0f){
        iter = get_iterations_mandelbrot();
    }
    else if (fractalType == 1.0f){
        iter = get_iterations_julia();
    }

    if (iter == MAX_ITERATIONS)
    {
        gl_FragDepth = 0.0f;
        return vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }
 
    float iterations = float(iter) / MAX_ITERATIONS;
    gl_FragDepth = iterations;
 
    float fraction = 0.0f;
    if (iterations < color_ranges[1])
    {
        fraction = (iterations - color_ranges[0]) / (color_ranges[1] - color_ranges[0]);
        return mix(color_0, color_1, fraction);
    }
    else if(iterations < color_ranges[2])
    {
        fraction = (iterations - color_ranges[1]) / (color_ranges[2] - color_ranges[1]);
        return mix(color_1, color_2, fraction);
    }
    else
    {
        fraction = (iterations - color_ranges[2]) / (color_ranges[3] - color_ranges[2]);
        return mix(color_2, color_3, fraction);
    }
}
 
void main()
{
    if(fractalType==2){
        fragColor = sierpinski();
    }else{
       fragColor = return_color(); 
    }
}