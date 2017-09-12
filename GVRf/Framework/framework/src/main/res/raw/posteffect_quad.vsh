// Copyright 2015 Samsung Electronics Co., LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifdef HAS_MULTIVIEW
#extension GL_OVR_multiview2 : enable
layout(num_views = 2) in;
#endif
in vec3 a_position;
in vec2 a_texcoord;

out vec2 v_scene_coord;
out vec2 v_overlay_coord;

void main() {
#ifdef HAS_MULTIVIEW
  gl_ViewID_OVR;
#endif
  v_scene_coord = a_texcoord.xy;
  
  // Textures loaded from an Android Bitmap are upside down relative to textures generated by EGL
  v_overlay_coord = vec2(a_texcoord.x, 1.0 - a_texcoord.y);

  gl_Position = vec4(a_position, 1.0);
}
