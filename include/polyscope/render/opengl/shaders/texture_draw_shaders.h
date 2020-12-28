#pragma once

#include "polyscope/render/opengl/gl_shaders.h"

namespace polyscope {
namespace render {
namespace backend_openGL3_glfw {

extern const ShaderStageSpecification TEXTURE_DRAW_VERT_SHADER;
extern const ShaderStageSpecification SPHEREBG_DRAW_VERT_SHADER;
extern const ShaderStageSpecification SPHEREBG_DRAW_FRAG_SHADER;
extern const ShaderStageSpecification PLAIN_TEXTURE_DRAW_FRAG_SHADER;
extern const ShaderStageSpecification DOT3_TEXTURE_DRAW_FRAG_SHADER;
extern const ShaderStageSpecification MAP3_TEXTURE_DRAW_FRAG_SHADER;
extern const ShaderStageSpecification COMPOSITE_PEEL;
extern const ShaderStageSpecification DEPTH_COPY;
extern const ShaderStageSpecification SCALAR_TEXTURE_COLORMAP;


// Rules
// extern const ShaderReplacementRule RULE_NAME;

// Shaders (which are used elsewhere)
extern const ShaderStageSpecification TEXTURE_DRAW_VERT_SHADER;
extern const ShaderStageSpecification SPHEREBG_DRAW_VERT_SHADER;

} // namespace backend_openGL3_glfw
} // namespace render
} // namespace polyscope
