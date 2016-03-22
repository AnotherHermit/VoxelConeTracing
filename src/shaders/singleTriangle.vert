///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#version 430

out vec2 exTexCoords;
 
void main()
{
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);
    exTexCoords.x = (x+1.0)*0.5;
    exTexCoords.y = (y+1.0)*0.5;
    gl_Position = vec4(x, y, 0, 1);
}