#version 330

// writes the pixel information in to the picking texture
// the object_index and draw_index are the same for all pixels (in the same draw call)

// In order to get the primitive index we use the built-in variable gl_PrimitiveID.
// This is a running index of the primitives which is automatically maintained by the system.
// gl_PrimitiveID can only be used in the GS and PS. If the GS is enabled and the FS
// wants to use gl_PrimitiveID, the GS must write gl_PrimitiveID into one of its output 
// variables and the FS must declare a variable by the same name for input. In our case
// we have no GS so we can simply use gl_PrimitiveID.

// The picking technique requires the application to update the draw index before each 
// draw call. 

uniform uint gDrawIndex;
uniform uint gObjectIndex;

out vec3 FragColor;

void main()
{
    FragColor = vec3(
                        float(gObjectIndex), 
                        float(gDrawIndex),
                        float(gl_PrimitiveID + 1)
                    );
}