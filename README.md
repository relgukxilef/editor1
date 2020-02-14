# game_engine1

## compiling a program

The functions `compile_shader` and `compile_program` handle reading files from disk, compiling, error checking and cleanup. `get_uniform_locations` allows querying multiple uniform locations at once. `bind_uniform_blocks` binds a list of uniform blocks.

`std::initializer_list`s can be used to specify multiple calls to the GL at once.

~~~~c++
// manage attribute indices by an enum
enum attributes : GLuint {
    position, color, selection, normal, texture_coordinates,
    texture_unit, texture_slice
};

// uniform block binding points enum for data shared between programs
enum uniform_block_bindings : GLuint {
    view_properties
};

// shader that can be shared between programs
GLuint fragment_utils = compile_shader(
    GL_FRAGMENT_SHADER, "shaders/utils.fs"
);

// compile the program
GLuint obj_program = compile_program(
    // specify vs, tcs, tes, gs and fs, leave unused shaders as nullptr
    "shaders/obj.vs", nullptr, nullptr, nullptr, "shaders/obj.fs",
    
    // additional shaders
    {fragment_utils},

    // bind attribute locations
    {
        {"position", position}, {"normal", normal},
        {"texture_coordinates", texture_coordinates},
        {"unit", texture_unit},
        {"slice", texture_slice},
    }
);

// get the uniform indices to set their values later
GLuint obj_model_matrix, obj_textures[4];
get_uniform_locations(obj_program, {
    {"model", &obj_model_matrix},
    {"textures[0]", &obj_textures[0]},
    {"textures[1]", &obj_textures[1]},
    {"textures[2]", &obj_textures[2]},
    {"textures[3]", &obj_textures[3]},
});

// bind the block of the program to the right binding point 
bind_uniform_blocks(obj_program, {
    {"view_properties", view_properties},
});
~~~~