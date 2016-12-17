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
#include "objects/uniform_block.h"
#include "glm/gtc/type_ptr.hpp"
#include <cctype>
#include <sstream>
#include "util/gvr_gl.h"

namespace gvr {

    UniformBlock::UniformBlock(const std::string& descriptor) :
            TotalSize(0),
            UniformData(NULL)
    {
        ownData = false;
        if (!descriptor.empty())
        {
            LOGE("setting descriptor %s", descriptor.c_str());
            setDescriptor(descriptor);
        }
    }

    UniformBlock::UniformBlock() :
            TotalSize(0),
            UniformData(NULL)
    {
        ownData = false;
    }

    bool UniformBlock::setInt(std::string name, int val)
    {
        int size = sizeof(int);
        char* data = getData(name, size);
        if (data != NULL)
        {
            *((int*) data) = val;
            setDirty();
            return true;
        }
        return false;
    }

    bool UniformBlock::setFloat(std::string name, float val) {
        int size = sizeof(float);
        char* data = getData(name, size);
        if (data != NULL)
        {
            *((float*) data) = val;
            setDirty();
            return true;
        }
        return false;
    }

    bool UniformBlock::setVec(std::string name, const float* val, int n)
    {
        int bytesize = n * sizeof(float);
        char* data = getData(name, bytesize);
        if (data != NULL)
        {
            memcpy(data, val, bytesize);
            setDirty();
            return true;
        }
        return false;
    }

    bool UniformBlock::setIntVec(std::string name, const int* val, int n)
    {
        int bytesize = n * sizeof(int);
        char* data = getData(name, bytesize);
        if (data != NULL)
        {
            memcpy(data, val, bytesize);
            setDirty();
            return true;
        }
        return false;
    }

    bool UniformBlock::setVec2(std::string name, const glm::vec2& val)
    {
        int bytesize = 2 * sizeof(float);
        float* data = (float*) getData(name, bytesize);
        if (data != NULL)
        {
            data[0] = val.x;
            data[1] = val.y;
            setDirty();
            return true;
        }
        return false;
    }

    bool UniformBlock::setVec3(std::string name, const glm::vec3& val)
    {
        int bytesize = 3 * sizeof(float);
        float* data = (float*) getData(name, bytesize);
        if (data != NULL)
        {
            data[0] = val.x;
            data[1] = val.y;
            data[2] = val.z;
            setDirty();
            return true;
        }
        return false;
    }

    bool UniformBlock::setVec4(std::string name, const glm::vec4& val)
    {
        int bytesize = 4 * sizeof(float);
        float* data = (float*) getData(name, bytesize);
        if (data != NULL)
        {
            data[0] = val.x;
            data[1] = val.y;
            data[2] = val.z;
            data[3] = val.w;
            setDirty();
            return true;
        }
        return false;
    }

    bool UniformBlock::setMat4(std::string name, const float* val)
    {
        int bytesize = 16 * sizeof(float);
        char* data = getData(name, bytesize);
        if (data != NULL)
        {
            memcpy(data, (val), bytesize);
            setDirty();
            return true;
        }
        return false;
    }

    const glm::vec2* UniformBlock::getVec2(std::string name) const
    {
        int size = 2 * sizeof(float);
        const char* data = getData(name, size);
        if (data != NULL)
            return (reinterpret_cast<const glm::vec2*>(data));
        return NULL;
    }

    const glm::vec3* UniformBlock::getVec3(std::string name) const
    {
        int size = 3 * sizeof(float);
        const char* data = getData(name, size);
        if (data != NULL)
            return (reinterpret_cast<const glm::vec3*> (data));
        return NULL;
    }

    const glm::vec4* UniformBlock::getVec4(std::string name) const
    {
        int size = 4 * sizeof(float);
        LOGE("UniformBlock get vec4 here %s", name.c_str());
        const char* data = getData(name, size);
        if (data != NULL)
            return (reinterpret_cast<const glm::vec4*> (data));
        return NULL;
    }

    int UniformBlock::getInt(std::string name) const
    {
        int size = sizeof(int);
        const char* data = getData(name, size);
        if (data != NULL)
            return *(reinterpret_cast<const int*> (data));
        return 0;
    }

    float UniformBlock::getFloat(std::string name) const
    {
        LOGE("UniformBlock good heregood%s", name.c_str());
        int size = sizeof(float);
        const char* data = getData(name, size);
        if (data != NULL)
            return *(reinterpret_cast<const float*> (data));
        return 0.0f;
    }

    bool UniformBlock::getIntVec(std::string name, int* val, int n) const
    {
        int size = n * sizeof(int);
        const char* data = getData(name, size);
        if (data != NULL)
        {
            memcpy((char*) val, data,size);
            return true;
        }
        LOGE("ERROR: UniformBlock element %s not found\n", name.c_str());
        return false;
    }

    bool UniformBlock::getVec(std::string name, float* val, int n) const
    {
        LOGE("UniformBlock bad here%s", name.c_str());
        int size =  n * sizeof(float);
        const char* data = getData(name, size);
        if (data != NULL)
        {
            memcpy((char*) val, data, n * sizeof(float));
            return true;
        }
        LOGE("ERROR: UniformBlock element %s not found\n", name.c_str());
        return false;
    }

    bool UniformBlock::getMat4(std::string name, glm::mat4& val) const
    {
        int bytesize = 16 * sizeof(float);
        const char* data = getData(name, bytesize);
        if (data != NULL)
        {
            val = glm::make_mat4((const float*) data);
            return true;
        }
        return false;
    }

    void  UniformBlock::parseDescriptor()
    {
        LOGD("UniformBlockNew: %s", Descriptor.c_str());
        const char* p = Descriptor.c_str();
        const char* type_start;
        int type_size;
        const char* name_start;
        int name_size;
        int offset = 0;
        const int VEC4_BOUNDARY = 15;
        TotalSize = 0;

        while (*p)
        {
            while (std::isspace(*p) || *p == ';'|| *p == ',')
                ++p;
            type_start = p;
            if (*p == 0)
                break;
            while (std::isalnum(*p))
                ++p;
            type_size = p - type_start;
            if (type_size == 0)
            {
                LOGE("UniformBlock: SYNTAX ERROR: expecting data type\n");
                break;
            }
            std::string type(type_start, type_size);
            while (std::isspace(*p))
                ++p;
            name_start = p;
            while (std::isalnum(*p) || (*p == '_'))
                ++p;
            name_size = p - name_start;

            // check if it is array
            int array_size = 1;

            if ((*p == '['))
            {
                ++p;
                while(std::isdigit(*p))
                {
                    array_size = array_size * 10 + (*p - '0');
                    ++p;
                }
                ++p;
            }
            if (name_size == 0)
            {
                LOGE("UniformBlock: SYNTAX ERROR: expecting uniform name\n");
                break;
            }
            Uniform uniform;
            std::string name(name_start, name_size);
            int byteSize = calcSize(type) * array_size;

            uniform.Name = name;
            uniform.Type = type;
            uniform.Offset = offset;
            uniform.Size = byteSize;                // get number of bytes

            if (byteSize == 0)
                continue;
            if ((offset + byteSize - (offset & ~0xF)) > 4 * sizeof(float))
            {
                offset = offset + 15 & ~0x0F;
            }
            std::pair<std::string, Uniform> pair(name, uniform);
            std::pair< std::map<std::string, Uniform>::iterator, bool > ret = UniformMap.insert(pair);
            if (!ret.second)
            {
                LOGE("UniformBlock: ERROR: element %s specified twice\n", name.c_str());
                continue;
            }
            LOGV("UniformBlock: %s offset=%d size=%d\n", name.c_str(), uniform.Offset, uniform.Size);
            offset += uniform.Size;
            TotalSize = uniform.Offset + uniform.Size;
        }
        if (TotalSize > 0)
        {
            TotalSize = (TotalSize + 15) & ~0x0F;
            UniformData = new char[TotalSize];
            memset(UniformData, 0, TotalSize);
            ownData = true;
        }
        else
        {
            LOGE("UniformBlock: ERROR: no uniform block allocated\n");
        }
    }

    int UniformBlock::calcSize(std::string type) const
    {
        if (type == "float") return sizeof(float);
        if (type == "float3") return 3 * sizeof(float);
        if (type == "float4") return 4 * sizeof(float);
        if (type == "float2") return 2 * sizeof(float);
        if (type == "int") return sizeof(int);
        if (type == "int2") return 2 * sizeof(int);
        if (type == "int3") return 3 * sizeof(int);
        if (type == "int4") return 4 * sizeof(int);
        if (type == "float2") return 2 * sizeof(int);
        if (type == "mat4") return 16 * sizeof(float);
        if (type == "mat3") return 12 * sizeof(float);
        LOGE("UniformBlock: SYNTAX ERROR: unknown type %s\n", type.c_str());
        return 0;
    }

    UniformBlock::Uniform* UniformBlock::getUniform(std::string name, int& bytesize)
    {
        auto it = UniformMap.find(name);
        if (it == UniformMap.end())
        {
            LOGE("ERROR: UniformBlock element %s not found\n", name.c_str());
            return NULL;
        }
        Uniform& u = it->second;
        if (u.Size < bytesize)
        {
            LOGE("ERROR: UniformBlock not const element %s is %d bytes, should be %d bytes\n", name.c_str(), bytesize, u.Size);
            return NULL;
        }
        bytesize = u.Size;
        LOGV("SHADER: UniformBlock not const element %s offset %d bytes,\n", name.c_str(), u.Offset);
        return &u;
    }

    const UniformBlock::Uniform* UniformBlock::getUniform(std::string name, int& bytesize) const
    {
        auto it = UniformMap.find(name);
        if (it == UniformMap.end())
        {
            LOGE("ERROR: UniformBlock element %s not found\n", name.c_str());
            return NULL;
        }
        const Uniform& u = it->second;
        if (u.Size < bytesize)
        {
            LOGE("ERROR: UniformBlock const element %s is %d bytes, should be %d bytes\n", name.c_str(), bytesize, u.Size);
            return NULL;
        }
        bytesize = u.Size;
        return &u;
    }

    const char* UniformBlock::getData(std::string name, int& bytesize) const
    {
        const Uniform* u = getUniform(name, bytesize);
        if (u == NULL)
            return NULL;
        char* data = (char*) UniformData;
        data += u->Offset;
        return data;
    }

    char* UniformBlock::getData(std::string name, int& bytesize)
    {
        Uniform* u = getUniform(name, bytesize);
        if (u == NULL)
            return NULL;
        char* data = (char*) UniformData;

        data += u->Offset;
        return data;
    }

    std::string UniformBlock::toString()
    {
        std::ostringstream os;
        char* data = (char*) UniformData;
        for (auto it = UniformMap.begin(); it != UniformMap.end(); ++it)
        {
            std::pair<std::string, Uniform> p = (*it);
            Uniform* u = &(p.second);
            os << p.first << ":" << u->Offset;
            for (int i = 0; i < u->Size / sizeof(float); i++)
            {
                char* d = data + u->Offset;
                os << " ";
                if (p.first[0] == 'i')
                {
                    os << *(((int*) d) + i);
                }
                else
                {
                    os << *(((float*) d) + i);
                }
            }
            os << ';' << std::endl;
        }
        return os.str();
    }

}