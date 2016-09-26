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

package org.gearvrf;

import java.util.HashMap;
import java.util.Map;

/**
 * Manages post-effect shaders, for modifying the texture holding the rendered
 * scene graph, before lens distortion is applied.
 * 
 * Get the singleton from {@link GVRContext#getPostEffectShaderManager()}
 */
public class GVRPostEffectShaderManager extends GVRShaderManager {

    GVRPostEffectShaderManager(GVRContext gvrContext) {
        super(gvrContext);
    }

    @Override
    public long addShader(String signature, String vertexShader, String fragmentShader)
    {
        return NativePostEffectShaderManager.addCustomPostEffectShader(getNative(),
                vertexShader, fragmentShader);
    }
}

class NativePostEffectShaderManager {
    static native long ctor();

    static native long delete(long postEffectShaderManager);

    static native int addCustomPostEffectShader(long postEffectShaderManager,
            String vertexShader, String fragmentShader);

    static native long getCustomPostEffectShader(long postEffectShaderManager,
            int id);
}
