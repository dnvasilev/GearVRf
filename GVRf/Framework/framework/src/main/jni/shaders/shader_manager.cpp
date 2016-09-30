#include "util/gvr_log.h"

#include "shaders/shader_manager.h"

namespace gvr {
    ShaderManager::~ShaderManager()
    {
        LOGE("SHADER: deleting ShaderManager");
        for (auto it = shadersByID.begin(); it != shadersByID.end(); ++it) {
            Shader *shader = it->second;
            delete shader;
        }
        shadersByID.clear();
        shadersBySignature.clear();
    }

    long ShaderManager::addShader(const std::string& signature,
          const std::string& uniformDescriptor,
          const std::string& textureDescriptor,
          const std::string& vertexDescriptor,
          const std::string& vertex_shader,
          const std::string& fragment_shader)
    {
        Shader* shader = findShader(signature);
        if (shader != NULL)
        {
            return shader->getShaderID();
        }
        std::lock_guard<std::mutex> lock(lock_);
        long id = ++latest_shader_id_;
        shader = new Shader(id, signature, uniformDescriptor, textureDescriptor, vertexDescriptor, vertex_shader, fragment_shader);
        shadersBySignature[signature] = shader;
        shadersByID[id] = shader;
        LOGD("SHADER: added shader %ld %s", id, signature.c_str());
        return id;
    }

    Shader* ShaderManager::findShader(const std::string& signature)
    {
        std::lock_guard<std::mutex> lock(lock_);
        auto it = shadersBySignature.find(signature);
        if (it != shadersBySignature.end())
        {
            Shader* shader = it->second;
            const std::string& sig = shader->signature();
            LOGD("SHADER: findShader %s -> %ld", sig.c_str(), shader->getShaderID());
            return shader;
        }
        else
        {
            return NULL;
        }
    }

    Shader* ShaderManager::getShader(long id)
    {
        std::lock_guard<std::mutex> lock(lock_);
        auto it = shadersByID.find(id);
        if (it != shadersByID.end())
        {
            Shader* shader = it->second;
            const std::string& sig = shader->signature();
            LOGD("SHADER: getShader %ld -> %s", id, sig.c_str());
            return shader;
        }
        else
        {
            LOGD("SHADER: getShader %ld NOT FOUND", id);
            return NULL;
        }
    }

    void ShaderManager::dump()
    {
        for (auto it = shadersByID.begin(); it != shadersByID.end(); ++it)
        {
            Shader* shader = it->second;
            long id = shader->getShaderID();
            const std::string& sig = shader->signature();
            int* data = (int*) shader;
            LOGD("SHADER: #%ld %s", id, sig.c_str());
        }
    }
}