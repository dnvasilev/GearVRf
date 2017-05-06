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
#include "objects/data_descriptor.h"
#include <sstream>
#include "util/gvr_log.h"

namespace gvr
{

    DataDescriptor::DataDescriptor(const std::string& descriptor) :
            mTotalSize(0),
            mIsDirty(false),
            mDescriptor(descriptor)
    {
        if (!descriptor.empty())
        {
            LOGE("setting descriptor %s", descriptor.c_str());
            parseDescriptor();
        }
        else
        {
            LOGE("DataDescriptor: Error: missing descriptor string");
        }
    }

    void DataDescriptor::forEachEntry(std::function<void(const DataEntry&)> func)
    {
        for (auto it = mLayout.begin(); it != mLayout.end(); ++it)
        {
            func(*it);
        }
    }

    void DataDescriptor::forEach(std::function<void(const std::string&, const std::string&, int)> func)
    {
        const char* p = mDescriptor.c_str();
        const char* type_start;
        int type_size;
        const char* name_start;
        int name_size;
        int index = 0;

        while (*p)
        {
            while (std::isspace(*p) || std::ispunct(*p))
                ++p;
            type_start = p;
            if (*p == 0)
                break;
            while (std::isalnum(*p))
                ++p;
            type_size = p - type_start;
            if (type_size == 0)
            {
                break;
            }
            std::string type(type_start, type_size);
            while (std::isspace(*p))
                ++p;
            name_start = p;
            while (std::isalnum(*p) || (*p == '_'))
                ++p;
            name_size = p - name_start;
            if (name_size == 0)
            {
                break;
            }
            std::string name(name_start, name_size);
            int size = calcSize(type);
            func(name, type, size);
            ++index;
        }
    }

    void  DataDescriptor::parseDescriptor()
    {
        int index = 0;
        forEach([this, index](const std::string& name, const std::string& type, int size) mutable
        {
            // check if it is array
            int array_size = 1;
            const char* p = name.c_str();
            int i = name.find('[');

            if (name.empty())
            {
                LOGE("UniformBlock: SYNTAX ERROR: expecting uniform name\n");
                return;
            }
            if (i > 0)
            {
                p += i + 1;
                while (std::isdigit(*p))
                {
                    array_size = array_size * 10 + (*p - '0');
                    ++p;
                }
                ++p;
            }
            DataEntry entry;
            short byteSize = calcSize(type);
            short offset = mTotalSize;

            entry.Type = makeShaderType(type, byteSize, array_size);
            byteSize *= array_size;
            entry.IsSet = false;
            entry.Name = name;
            entry.Index = index++;
            entry.Offset = offset;
            entry.Size = byteSize;

            if (byteSize == 0)
                return;
            if ((offset + byteSize - (offset & ~0xF)) > 4 * sizeof(float))
            {
                offset = offset + 15 & ~0x0F;
            }
            mLayout.push_back(entry);
            LOGV("UniformBlock: %s offset=%d size=%d\n", name.c_str(), entry.Offset, entry.Size);
            offset += entry.Size;
            mTotalSize = entry.Offset + entry.Size;
        });
        if (mTotalSize > 0)
        {
            mTotalSize = (mTotalSize + 15) & ~0x0F;
        }
    }

    std::string DataDescriptor::makeShaderType(const std::string& type, int byteSize, int arraySize)
    {
        std::ostringstream stream;

        if ((byteSize > 4) && (byteSize <= 16))
        {
            if (type[0] == 'f')
            {
                stream << "vec" << (byteSize / 4);
            }
            else if (type[0] == 'i')
            {
                stream << "vec" << (byteSize / 4);
            }
            else
            {
                stream << type;
            }
        }
        else
        {
            stream << type;
        }
        if (arraySize > 1)
        {
            stream << "[" << arraySize << "]";
        }
        return stream.str();
    }

    short DataDescriptor::calcSize(const std::string &type) const
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
        LOGE("DataDescriptor: SYNTAX ERROR: unknown type %s\n", type.c_str());
        return 0;
    }

    const DataDescriptor::DataEntry* DataDescriptor::find(const std::string &name) const
    {
        for (auto it = mLayout.begin(); it != mLayout.end(); ++it)
        {
            if (it->Name == name)
            {
                return &(*it);
            }
        }
        return nullptr;
    }

    DataDescriptor::DataEntry* DataDescriptor::find(const std::string &name)
    {
        for (auto it = mLayout.begin(); it != mLayout.end(); ++it)
        {
            if (it->Name == name)
            {
                return &(*it);
            }
        }
        return nullptr;
    }

    int DataDescriptor::getByteSize(const std::string &name) const
    {
        const DataEntry* e = find(name);
        return e ? e->Size : 0;
    }

}
