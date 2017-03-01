/* Copyright 2015 Samsung Electronics Co., LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/***************************************************************************
 * A shader which an user can add in run-time.
 ***************************************************************************/

#include "vulkan/vulkan_shader.h"
#include "objects/shader_data.h"
#include "engine/renderer/vulkan_renderer.h"
#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>

namespace gvr {

VulkanShader::VulkanShader(int id,
               const std::string& signature,
               const std::string& uniformDescriptor,
               const std::string& textureDescriptor,
               const std::string& vertexDescriptor,
               const std::string& vertexShader,
               const std::string& fragmentShader)
    : Shader(id, signature, uniformDescriptor, textureDescriptor, vertexDescriptor, vertexShader, fragmentShader) { }

void VulkanShader::initialize(Mesh* mesh)
{
}


VulkanShader::~VulkanShader() { }


void static fillDelims(std::unordered_map<std::string, int> &delims, std::string &delimsTypes)
{
    for(uint i = 0; i < delimsTypes.length(); i++){
        delims[delimsTypes.substr(i,1)] = 1;
    }
}

std::vector<std::string> static getTokens(std::string &input)
{
    std::vector <std::string> tokens;
    std::unordered_map<std::string, int> delims;
    std::string delimiters = " ;+-/*%()<>!={}\n";
    fillDelims(delims, delimiters);

    int prev = 0;
    for(uint i = 0; i < input.length(); i++){
        if(delims[input.substr(i, 1)]){
            tokens.push_back(input.substr(prev, i-prev));
            tokens.push_back(input.substr(i, 1));
            prev = i+1;
        }
        else{

        }
    }

    return tokens;
}

    void static insertBindingPoints(std::string &shader)
    {
        std::unordered_map<std::string, std::string> uniformBindings;
        uniformBindings["Material_ubo"] = "1";
        uniformBindings["Transform_ubo"] = "0";
        std::vector<std::string> tokens = getTokens(shader);

        for(uint i = 0; i < tokens.size(); ++i)
        {
            if(tokens[i] == "std140")
            {
                for(uint j = i+1; j < tokens.size(); ++j)
                {
                    std::unordered_map<std::string, std::string>::const_iterator found = uniformBindings.find(tokens[j]);
                    if(found != uniformBindings.end())
                    {
                        tokens[i] += ", binding = " + uniformBindings[tokens[j]];
                        i = j + 1;
                        break;
                    }
                }
            }
        }
        shader = "";

        for(uint i = 0; i < tokens.size(); ++i)
        {
            shader += tokens[i];
        }
    }

    std::vector<uint32_t> VulkanShader::CompileVulkanShader(const std::string& shaderName, ShaderType shaderTypeID, std::string& shaderContents)
    {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        shaderc_shader_kind shaderType;

        switch (shaderTypeID)
        {
            case VERTEX_SHADER:
                shaderType = shaderc_glsl_default_vertex_shader;
                break;
            case FRAGMENT_SHADER:
                shaderType = shaderc_glsl_default_fragment_shader;
                break;
        }

        // Modify GL Shader to VK Shader
        std::string append = "400 \n #extension GL_ARB_separate_shader_objects : enable \n #extension GL_ARB_shading_language_420pack : enable \n";
        std::size_t found = shaderContents.find("300 es");
        if (found != std::string::npos)
        {
            shaderContents.replace(found, 6, append);
        }

        // Inserting Binding points
        insertBindingPoints(shaderContents);

        shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(shaderContents.c_str(),
                                                                         shaderContents.size(),
                                                                         shaderType,
                                                                         shaderName.c_str(),
                                                                         options);

        if (module.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            LOGI("Vulkan shader unable to compile : %s", module.GetErrorMessage().c_str());
        }

        std::vector<uint32_t> result(module.cbegin(), module.cend());
        return result;
    }
} /* namespace gvr */
