enum attributes : GLuint {
    vertex_position, instance_position // attribute indices
};

GLuint vertex_position_buffer, instance_position_buffer; // vertex buffers
GLuint draw_indirect_buffer; 
GLuint cube_mesh, sphere_mesh; // mesh vertex offsets
GLuint cube_instances, sphere_instances; // mesh instance offsets

GLuint vao = create_vertex_array(
    { // vertex attributes
        {{
            {vertex_position, 3, GL_FLOAT, GL_FALSE, 0}, 
        }, 3 * sizeof(float), GL_STATIC_DRAW, &vertex_position_buffer},
    }, { // instance attributes
        {{
            {instance_position, 4, GL_FLOAT, GL_FALSE, 0}, 
        }, 4 * sizeof(float), GL_DYNAMIC_DRAW, &instance_position_buffer},
    }, { // commands
        {
            max_cube_vertex_count, max_sphere_instances, 
            &sphere_mesh, &sphere_instances
        }, {
            max_cube_vertex_count, max_cube_instances, 
            &cube_mesh, &cube_instances
        }, 
    }, 
    &draw_indirect_buffer,
    1 // attribute divisor
);

// TODO: handle case where multiple commands draw the same mesh
// TODO: maybe even cases where commands are overlapping