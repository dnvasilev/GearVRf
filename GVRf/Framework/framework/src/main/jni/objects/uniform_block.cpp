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

namespace gvr
{
    UniformBlock::UniformBlock(const std::string &descriptor, int bindingPoint) :
            TotalSize(0),
            bindingPoint_(bindingPoint),
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

    bool UniformBlock::setInt(const std::string &name, int val)
    {
        int size = sizeof(int);
        char *data = getData(name, size);
        if (data != NULL)
        {
            *((int *) data) = val;
            return true;
        }
        return false;
    }

    bool UniformBlock::setFloat(const std::string &name, float val)
    {
        int size = sizeof(float);
        char *data = getData(name, size);
        if (data != NULL)
        {
            *((float *) data) = val;
            return true;
        }
        return false;
    }

    bool UniformBlock::setFloatVec(const std::string &name, const float *val, int n)
    {
        int bytesize = n * sizeof(float);
        char *data = getData(name, bytesize);
        if (data != NULL)
        {
            memcpy(data, val, bytesize);
            return true;
        }
        return false;
    }

    bool UniformBlock::setIntVec(const std::string &name, const int *val, int n)
    {
        int bytesize = n * sizeof(int);
        char *data = getData(name, bytesize);
        if (data != NULL)
        {
            memcpy(data, val, bytesize);
            return true;
        }
        return false;
    }

    bool UniformBlock::setVec2(const std::string &name, const glm::vec2 &val)
    {
        int bytesize = 2 * sizeof(float);
        float *data = (float *) getData(name, bytesize);
        if (data != NULL)
        {
            data[0] = val.x;
            data[1] = val.y;
            return true;
        }
        return false;
    }

    bool UniformBlock::setVec3(const std::string &name, const glm::vec3 &val)
    {
        int bytesize = 3 * sizeof(float);
        float *data = (float *) getData(name, bytesize);
        if (data != NULL)
        {
            data[0] = val.x;
            data[1] = val.y;
            data[2] = val.z;
            return true;
        }
        return false;
    }

    bool UniformBlock::setVec4(const std::string &name, const glm::vec4 &val)
    {
        int bytesize = 4 * sizeof(float);
        float *data = (float *) getData(name, bytesize);
        if (data != NULL)
        {
            data[0] = val.x;
            data[1] = val.y;
            data[2] = val.z;
            data[3] = val.w;
            return true;
        }
        return false;
    }

    bool UniformBlock::setMat4(const std::string &name, const glm::mat4 &val)
    {
        const float *mtxdata = glm::value_ptr(val);
        int bytesize = 16 * sizeof(float);
        char *data = getData(name, bytesize);
        if (data != NULL)
        {
            memcpy(data, mtxdata, bytesize);
            return true;
        }
        return false;
    }

    const glm::vec2 *UniformBlock::getVec2(const std::string &name) const
    {
        int size = 2 * sizeof(float);
        const char *data = getData(name, size);
        if (data != NULL)
            return (reinterpret_cast<const glm::vec2 *>(data));
        return NULL;
    }

    const glm::vec3 *UniformBlock::getVec3(const std::string &name) const
    {
        int size = 3 * sizeof(float);
        const char *data = getData(name, size);
        if (data != NULL)
            return (reinterpret_cast<const glm::vec3 *> (data));
        return NULL;
    }

    const glm::vec4 *UniformBlock::getVec4(const std::string &name) const
    {
        int size = 4 * sizeof(float);
        const char *data = getData(name, size);
        if (data != NULL)
            return (reinterpret_cast<const glm::vec4 *> (data));
        return NULL;
    }

    bool UniformBlock::getInt(const std::string &name, int &v) const
    {
        int size = sizeof(int);
        const char *data = getData(name, size);
        if (data != NULL)
        {
            v = *(reinterpret_cast<const int *> (data));
            return true;
        }
        return false;
    }

    bool UniformBlock::getFloat(const std::string &name, float &v) const
    {
        int size = sizeof(float);
        const char *data = getData(name, size);
        if (data != NULL)
        {
            v = *(reinterpret_cast<const float *> (data));
            return true;
        }
        return false;
    }

    bool UniformBlock::getIntVec(const std::string &name, int *val, int n) const
    {
        int size = n * sizeof(int);
        const char *data = getData(name, size);
        if (data != NULL)
        {
            memcpy((char *) val, data, size);
            return true;
        }
        LOGE("ERROR: UniformBlock element %s not found\n", name.c_str());
        return false;
    }

    bool UniformBlock::getFloatVec(const std::string &name, float *val, int n) const
    {
        int size = n * sizeof(float);
        const char *data = getData(name, size);
        if (data != NULL)
        {
            memcpy((char *) val, data, n * sizeof(float));
            return true;
        }
        LOGE("ERROR: UniformBlock element %s not found\n", name.c_str());
        return false;
    }

    bool UniformBlock::getMat4(const std::string &name, glm::mat4 &val) const
    {
        int bytesize = 16 * sizeof(float);
        const char *data = getData(name, bytesize);
        if (data != NULL)
        {
            val = glm::make_mat4((const float *) data);
            return true;
        }
        return false;
    }

    void  UniformBlock::parseDescriptor()
    {
        LOGD("UniformBlockNew: %s", Descriptor.c_str());
        const char *p = Descriptor.c_str();
        const char *type_start;
        int type_size;
        const char *name_start;
        int name_size;
        short offset = 0;
        const int VEC4_BOUNDARY = 15;
        TotalSize = 0;

        while (*p)
        {
            while (std::isspace(*p) || *p == ';' || *p == ',')
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
                while (std::isdigit(*p))
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
            short byteSize = calcSize(type) * array_size;

            uniform.IsSet = false;
            uniform.Name = name;
            uniform.Type = type;
            uniform.Offset = offset;
            uniform.Size = byteSize;

            if (byteSize == 0)
                continue;
            if ((offset + byteSize - (offset & ~0xF)) > 4 * sizeof(float))
            {
                offset = offset + 15 & ~0x0F;
            }
            std::pair<std::string, Uniform> pair(name, uniform);
            std::pair<std::map<std::string, Uniform>::iterator, bool> ret = UniformMap.insert(pair);
            if (!ret.second)
            {
                LOGE("UniformBlock: ERROR: element %s specified twice\n", name.c_str());
                continue;
            }
            LOGV("UniformBlock: %s offset=%d size=%d\n", name.c_str(), uniform.Offset,
                 uniform.Size);
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

    short UniformBlock::calcSize(const std::string &type) const
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

    UniformBlock::Uniform *UniformBlock::getUniform(const std::string &name, int &bytesize)
    {
        auto it = UniformMap.find(name);
        if (it == UniformMap.end())
        {
            LOGE("ERROR: UniformBlock element %s not found\n", name.c_str());
            return NULL;
        }
        Uniform &u = it->second;
        if (u.Size < bytesize)
        {
            LOGE("ERROR: UniformBlock not const element %s is %d bytes, should be %d bytes\n",
                 name.c_str(), bytesize, u.Size);
            return NULL;
        }
        bytesize = u.Size;
        LOGV("SHADER: UniformBlock not const element %s offset %d bytes,\n", name.c_str(),
             u.Offset);
        return &u;
    }

    int UniformBlock::getByteSize(const std::string &name) const
    {
        auto it = UniformMap.find(name);
        if (it == UniformMap.end())
        {
            return 0;
        }
        const Uniform &u = it->second;
        return u.Size;
    }

    const UniformBlock::Uniform *UniformBlock::getUniform(const std::string &name,
                                                          int &bytesize) const
    {
        auto it = UniformMap.find(name);
        if (it == UniformMap.end())
        {
            LOGE("ERROR: UniformBlock element %s not found\n", name.c_str());
            return NULL;
        }
        const Uniform &u = it->second;
        if (u.Size < bytesize)
        {
            LOGE("ERROR: UniformBlock const element %s is %d bytes, should be %d bytes\n",
                 name.c_str(), bytesize, u.Size);
            return NULL;
        }
        bytesize = u.Size;
        return &u;
    }

    const char *UniformBlock::getData(const std::string &name, int &bytesize) const
    {
        const Uniform *u = getUniform(name, bytesize);
        if (u == NULL)
            return NULL;
        char *data = (char *) UniformData;
        data += u->Offset;
        return data;
    }

    char *UniformBlock::getData(const std::string &name, int &bytesize)
    {
        Uniform *u = getUniform(name, bytesize);
        if (u == NULL)
            return NULL;
        char *data = (char *) UniformData;

        data += u->Offset;
        u->IsSet = true;
        return data;
    }

    std::string UniformBlock::toString()
    {
        std::ostringstream os;
        char *data = (char *) UniformData;
        for (auto it = UniformMap.begin(); it != UniformMap.end(); ++it)
        {
            std::pair<std::string, Uniform> p = (*it);
            Uniform *u = &(p.second);
            os << p.first << ":" << u->Offset;
            for (int i = 0; i < u->Size / sizeof(float); i++)
            {
                char *d = data + u->Offset;
                os << " ";
                if (p.first[0] == 'i')
                {
                    os << *(((int *) d) + i);
                }
                else
                {
                    os << *(((float *) d) + i);
                }
            }
            os << ';' << std::endl;
        }
        return os.str();
    }
}
