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

import org.gearvrf.utility.ResourceCache;
import org.joml.Matrix4f;
import org.joml.Vector4f;

import java.util.HashMap;

/**
 * Causes a shadow map to be rendered from the viewpoint
 * of the scene object which owns the shadow map.
 * A shadow map is only rendered if a light is
 * attached to the owning scene object.
 * All of the shadow maps are kept in a single
 * texture array and they must all be the same size.
 * This array is created the first time a GVRShadowMap
 * component is created.
 * @see GVRRenderTarget
 * @see GVRRenderTextureArray
 */
public class GVRShadowMap extends GVRRenderTarget
{
    /**
     * When the application is restarted we recreate the render texture array
     * since all of the GL textures have been deleted.
     */
    static
    {
        GVRContext.addResetOnRestartHandler(new Runnable()
        {
            @Override
            public void run()
            {
                sShadowMaps = null;
                sShadowMaterial = null;
            }
        });
    }

    /**
     * Constructs a shadow map using the given camera.
     *
     * @param ctx GVRContext to associate the shadow map with.
     * @param camera GVRCamera used to cast shadow
     */
    public GVRShadowMap(GVRContext ctx, GVRCamera camera)
    {
        super(ctx, NativeShadowMap.ctor(getShadowMaterial(ctx).getNative()));
        mShadowMatrix = new Matrix4f();
        mTemp = new Vector4f();
        mTempMtx = new float[16];
        if (sShadowMaps == null)
        {
            sShadowMaps = new GVRRenderTextureArray(ctx, 1024, 1024, 2, 4);
            sBiasMatrix = new Matrix4f();
            sBiasMatrix.scale(0.5f);
            sBiasMatrix.setTranslation(0.5f, 0.5f, 0.5f);
        }
        setTexture(sShadowMaps);
        setCamera(camera);
    }

    /**
     * Adds an orthographic camera constructed from the designated
     * perspective camera to describe the shadow projection.
     * The field of view and aspect ration of the perspective
     * camera are used to obtain the view volume of the
     * orthographic camera. This type of camera is used
     * for shadows generated by direct lights at infinite distance.
     * @param centerCam GVRPerspectiveCamera to derive shadow projection from
     * @return Orthographic camera to use for shadow casting
     * @see GVRDirectLight
     */
    static GVROrthogonalCamera makeOrthoShadowCamera(GVRPerspectiveCamera centerCam)
    {
        GVROrthogonalCamera shadowCam = new GVROrthogonalCamera(centerCam.getGVRContext());
        float near = centerCam.getNearClippingDistance();
        float far = centerCam.getFarClippingDistance();
        float fovy = (float) Math.toRadians(centerCam.getFovY());
        float h = (float) (Math.atan(fovy / 2.0f) * far) / 2.0f;

        shadowCam.setLeftClippingDistance(-h);
        shadowCam.setRightClippingDistance(h);
        shadowCam.setTopClippingDistance(h);
        shadowCam.setBottomClippingDistance(-h);
        shadowCam.setNearClippingDistance(near);
        shadowCam.setFarClippingDistance(far);
        return shadowCam;
    }

    /**
     * Adds a perspective camera constructed from the designated
     * perspective camera to describe the shadow projection.
     * This type of camera is used for shadows generated by spot lights.
     * @param centerCam GVRPerspectiveCamera to derive shadow projection from
     * @param coneAngle spot light cone angle
     * @return Perspective camera to use for shadow casting
     * @see GVRSpotLight
     */
    static GVRPerspectiveCamera makePerspShadowCamera(GVRPerspectiveCamera centerCam, float coneAngle)
    {
        GVRPerspectiveCamera camera = new GVRPerspectiveCamera(centerCam.getGVRContext());
        float near = centerCam.getNearClippingDistance();
        float far = centerCam.getFarClippingDistance();

        camera.setNearClippingDistance(near);
        camera.setFarClippingDistance(far);
        camera.setFovY((float) Math.toDegrees(coneAngle));
        camera.setAspectRatio(1.0f);
        return camera;
    }

    /**
     * Sets the shadow matrix for the spot light from the input model/view
     * matrix and the shadow camera projection matrix.
     * @param modelMtx  light model transform (to world coordinates)
     * @param light     spot light component to update
     */
    void setPerspShadowMatrix(Matrix4f modelMtx, GVRLightBase light)
    {
        GVRPerspectiveCamera camera = (GVRPerspectiveCamera) getCamera();

        if (camera == null)
        {
            return;
        }
        float angle = light.getFloat("outer_cone_angle");
        float near = camera.getNearClippingDistance();
        float far = camera.getFarClippingDistance();

        angle = (float) Math.acos(angle) * 2.0f;
        modelMtx.invert();
        modelMtx.get(mTempMtx);
        camera.setViewMatrix(mTempMtx);
        camera.setFovY((float) Math.toDegrees(angle));
        mShadowMatrix.setPerspective(angle, 1.0f, near, far);
        mShadowMatrix.mul(modelMtx);
        sBiasMatrix.mul(mShadowMatrix, mShadowMatrix);
        mShadowMatrix.getColumn(0, mTemp);
        light.setVec4("sm0", mTemp.x, mTemp.y, mTemp.z, mTemp.w);
        mShadowMatrix.getColumn(1, mTemp);
        light.setVec4("sm1", mTemp.x, mTemp.y, mTemp.z, mTemp.w);
        mShadowMatrix.getColumn(2, mTemp);
        light.setVec4("sm2", mTemp.x, mTemp.y, mTemp.z, mTemp.w);
        mShadowMatrix.getColumn(3, mTemp);
        light.setVec4("sm3", mTemp.x, mTemp.y, mTemp.z, mTemp.w);
    }

    /**
     * Sets the direct light shadow matrix for the light from the input model/view
     * matrix and the shadow camera projection matrix.
     * @param modelMtx  light model transform (to world coordinates)
     * @param light     direct light component to update
     */
    void setOrthoShadowMatrix(Matrix4f modelMtx, GVRLightBase light)
    {
        GVROrthogonalCamera camera = (GVROrthogonalCamera) getCamera();
        if (camera == null)
        {
            return;
        }

        float w = camera.getRightClippingDistance() - camera.getLeftClippingDistance();
        float h = camera.getTopClippingDistance() - camera.getBottomClippingDistance();
        float near = camera.getNearClippingDistance();
        float far = camera.getFarClippingDistance();

        modelMtx.invert();
        modelMtx.get(mTempMtx);
        camera.setViewMatrix(mTempMtx);
        mShadowMatrix.setOrthoSymmetric(w, h, near, far);
        mShadowMatrix.mul(modelMtx);
        sBiasMatrix.mul(mShadowMatrix, mShadowMatrix);
        mShadowMatrix.getColumn(0, mTemp);
        light.setVec4("sm0", mTemp.x, mTemp.y, mTemp.z, mTemp.w);
        mShadowMatrix.getColumn(1, mTemp);
        light.setVec4("sm1", mTemp.x, mTemp.y, mTemp.z, mTemp.w);
        mShadowMatrix.getColumn(2, mTemp);
        light.setVec4("sm2", mTemp.x, mTemp.y, mTemp.z, mTemp.w);
        mShadowMatrix.getColumn(3, mTemp);
        light.setVec4("sm3", mTemp.x, mTemp.y, mTemp.z, mTemp.w);
    }

    /**
     * Gets the shadow material used in constructing shadow maps.
     * <p>
     * Adds the shadow mapping depth shaders to the shader manager.
     * There are two variants - one for skinned and one for non-skinned meshes.
     * @return shadow map material
     */
    static GVRMaterial getShadowMaterial(GVRContext ctx)
    {
        if (sShadowMaterial == null)
        {
            GVRShaderId depthShader = ctx.getMaterialShaderManager().getShaderType(GVRDepthShader.class);
            sShadowMaterial = new GVRMaterial(ctx, depthShader);
            GVRShader shaderTemplate = depthShader.getTemplate(ctx);
            shaderTemplate.bindShader(ctx, sShadowMaterial, "float3 a_position float3 a_normal");
            shaderTemplate.bindShader(ctx, sShadowMaterial, "float3 a_position float3 a_normal float4 a_bone_weights int4 a_bone_indices");
        }
        return sShadowMaterial;
    }

    protected Matrix4f mShadowMatrix;
    protected Vector4f mTemp;
    protected float[] mTempMtx;
    static Matrix4f sBiasMatrix = null;
    static GVRRenderTextureArray sShadowMaps = null;
    static GVRMaterial sShadowMaterial = null;
}

class NativeShadowMap
{
    static native long ctor(long material);
}