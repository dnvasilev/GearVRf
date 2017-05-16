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

#include "gl/gl_shader.h"
#include "objects/mesh.h"
#include "gl/gl_material.h"
#include "engine/renderer/renderer.h"

namespace gvr {

    GLShader::GLShader(int id,
               const std::string& signature,
               const std::string& uniformDescriptor,
               const std::string& textureDescriptor,
               const std::string& vertexDescriptor,
               const std::string& vertexShader,
               const std::string& fragmentShader)
    : Shader(id, signature, uniformDescriptor, textureDescriptor, vertexDescriptor, vertexShader, fragmentShader),
      mProgram(NULL)
{ }


GLShader::~GLShader()
{
    if (mProgram)
    {
        delete mProgram;
    }
}
void getTokens(std::unordered_map<std::string, int>& tokens, std::string& line)
{
    std::string delimiters = " ;+-/*%()<>!={}\n";
    std::unordered_set<char>delim;
    for(int i=0; i<delimiters.length(); i++){
        delim.insert(delimiters[i]);
    }
    int start  =0;
    for(int i=0; i<line.length(); i++){
        if(delim.find(line[i])!= delim.end()){
            if((i-start) > 0)
                tokens[line.substr(start, i-start)] = start;
            start = i+1;
        }
    }
}

void modifyShader(std::string& shader)
{
    std::istringstream shaderStream(shader);
    std::string line;
    std::getline(shaderStream, line);
    std::string mod_shader;
    mod_shader += "#version 300 es \n";

    std::unordered_map<std::string, int>::iterator it;
    std::unordered_map<std::string, int>::iterator it1;

    while (std::getline(shaderStream, line)) {

        if (line.find("GL_ARB_separate_shader_objects") != std::string::npos || line.find("GL_ARB_shading_language_420pack") != std::string::npos)
            continue;
        std::unordered_map<std::string, int> tokens;
        getTokens(tokens, line);

        if((it = tokens.find("uniform"))!=tokens.end()){
            int pos = tokens["layout"];
            mod_shader += ((pos > 0) ? line.substr((0,pos)) :"") + ((tokens.find("sampler")) == tokens.end() ? "layout (std140) " : "") + line.substr(it->second) + "\n";
        }
        else if((it = tokens.find("layout"))!=tokens.end()){
            it1 = tokens.find("in");
            if(it1 == tokens.end())
                it1 = tokens.find("out");
            int pos = it->second;

            mod_shader += ((pos > 0) ? line.substr(0,pos) :"") + line.substr(it1->second) + "\n";
        }
        else
            mod_shader += line + "\n";
    }
    shader = mod_shader;
}

void GLShader::convertToGLShaders()
{
    if(vertexShader_.find("#version 400")==std::string::npos)
        return;
    modifyShader(vertexShader_);
    modifyShader(fragmentShader_);

}

void GLShader::initialize()
{
    std::string modified_frag_shader;
    if (fragmentShader_.find("samplerExternalOES")!= std::string::npos)
    {
        std::istringstream iss(fragmentShader_.c_str());
        const char* extensions = (const char*) glGetString(GL_EXTENSIONS);
        std::string extension_string;
        if(strstr(extensions, "GL_OES_EGL_image_external_essl3"))
        {
            extension_string = "#extension GL_OES_EGL_image_external_essl3 : require \n";
        }
        else
        {
            extension_string = "#extension GL_OES_EGL_image_external : require\n";
        }
        std::string line;
        while (std::getline(iss, line))
        {
            if(line.find("GL_OES_EGL_image_external") != std::string::npos)
            {
                modified_frag_shader = modified_frag_shader + extension_string + "\n";
            }
            else
            {
                modified_frag_shader = modified_frag_shader + line + "\n";
            }
        }
    }
    else
    {
        modified_frag_shader = fragmentShader_;
    }

    mProgram = new GLProgram(vertexShader_.c_str(), modified_frag_shader.c_str());
    if (use_multiview && !(strstr(vertexShader_.c_str(), "gl_ViewID_OVR")
                           && strstr(vertexShader_.c_str(), "GL_OVR_multiview2")
                           && strstr(vertexShader_.c_str(), "GL_OVR_multiview2")))
    {
        std::string error = "Your shaders are not multiview";
        LOGE("Your shaders are not multiview");
        throw error;
    }
    vertexShader_.clear();
    fragmentShader_.clear();
}

bool GLShader::useShader()
{
    if (nullptr == mProgram)
    {
        initialize();
    }
    GLint programID = getProgramId();
    if (programID <= 0)
    {
        LOGE("SHADER: shader could not be generated %s", signature_.c_str());
        return false;
    }
    if (LOG_SHADER) LOGV("SHADER: rendering with program %d", programID);
    glUseProgram(programID);
    return true;
}

int GLShader::bindTextures(GLMaterial* material)
{
    int texUnit = 0;
    bool fail = false;
    int ntex = material->getNumTextures();

    mTextureLocs.resize(ntex, -1);
    material->forEachTexture([this, fail, texUnit, material](const std::string& texname, Texture* tex) mutable
    {
         if (mTextures.find(texname) <= 0)
         {
             LOGV("SHADER: program %d texture %s not used by shader", getProgramId(), texname.c_str());
             return;
         }
         int loc = mTextureLocs[texUnit];
         if (loc == -1)
         {
             loc = glGetUniformLocation(getProgramId(), texname.c_str());
             if (loc >= 0)
             {
                 mTextureLocs[texUnit] = loc;
                 LOGV("SHADER: program %d texture %s loc %d", getProgramId(), texname.c_str(), loc);
             }
             else
             {
                 fail = true;
                 LOGE("SHADER: texture %s has no location in shader %d", texname.c_str(), getProgramId());
                 return;
             }
         }
         material->bindTexture(tex, texUnit++, loc);
    });
    if (!fail)
    {
        return texUnit;
    }
    return -1;
}
} /* namespace gvr */
