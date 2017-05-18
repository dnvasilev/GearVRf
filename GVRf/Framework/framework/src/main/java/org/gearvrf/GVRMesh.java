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

import static org.gearvrf.utility.Assert.*;

import java.nio.CharBuffer;
import java.nio.FloatBuffer;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.gearvrf.utility.Exceptions;
import org.gearvrf.utility.Log;

/**
 * Describes an indexed triangle mesh as a set of shared vertices with integer
 * indices for each triangle.
 * 
 * Usually each mesh vertex may have a positions, normal and texture coordinate.
 * Skinned mesh vertices will also have bone weights and indices.
 * If the mesh uses a normal map for lighting, it will have tangents
 * and bitangents as well. These vertex components correspond to vertex
 * attributes in the OpenGL vertex shader.
 */
public class GVRMesh extends GVRHybridObject implements PrettyPrint {
    private static final String TAG = GVRMesh.class.getSimpleName();

    protected GVRVertexBuffer mVertices;
    protected GVRIndexBuffer mIndices;
    protected List<GVRBone> mBones = new ArrayList<GVRBone>();
    protected GVRVertexBoneData mVertexBoneData;

    public GVRMesh(GVRContext gvrContext) {
        this(gvrContext, "float3 a_position float2 a_texcoord float3 a_normal ");
    }

    public GVRMesh(GVRVertexBuffer vbuffer, GVRIndexBuffer ibuffer)
    {
        super(vbuffer.getGVRContext(), NativeMesh.ctorBuffers(vbuffer.getNative(), (ibuffer != null) ? ibuffer.getNative() : 0L));
        mVertices = vbuffer;
        mIndices = ibuffer;
        setBones(new ArrayList<GVRBone>());
        mVertexBoneData = new GVRVertexBoneData(vbuffer.getGVRContext(), this);
    }

    public GVRMesh(GVRContext gvrContext, String vertexDescriptor) {
        this(new GVRVertexBuffer(gvrContext, vertexDescriptor, 0), null);
    }

    /**
     * Get the 3D vertices of the mesh. Each vertex is represented as a packed
     * {@code float} triplet:
     * <p>
     * <code>
     *     { x0, y0, z0, x1, y1, z1, x2, y2, z2, ... }
     * </code>
     * 
     * @return Array with the packed vertex data.
     */
    public float[] getVertices() {
        return mVertices.getFloatVec("a_position").array();
    }

    /**
     * Sets the 3D vertices of the mesh. Each vertex is represented as a packed
     * {@code float} triplet:
     * <p>
     * <code>{ x0, y0, z0, x1, y1, z1, x2, y2, z2, ...}</code>
     * 
     * @param vertices
     *            Array containing the packed vertex data.
     */
    public void setVertices(float[] vertices) {
        mVertices.setFloatVec("a_position", vertices);
    }

    public GVRVertexBuffer getVertexBuffer() { return mVertices; }

    public GVRIndexBuffer getIndexBuffer() { return mIndices; }

    public void setVertexBuffer(GVRVertexBuffer vbuf)
    {
        if (vbuf == null)
        {
            throw new IllegalArgumentException("Vertex buffer cannot be null");
        }
        mVertices = vbuf;
        NativeMesh.setVertexBuffer(getNative(), vbuf.getNative());
    }

    public void setIndexBuffer(GVRIndexBuffer ibuf)
    {
        mIndices = ibuf;
        NativeMesh.setIndexBuffer(getNative(), (ibuf != null) ? ibuf.getNative() : 0L);
    }

    /**
     * Get the normal vectors of the mesh. Each normal vector is represented as
     * a packed {@code float} triplet:
     * <p>
     * <code>{ x0, y0, z0, x1, y1, z1, x2, y2, z2, ...}</code>
     * 
     * @return Array with the packed normal data.
     */
    public float[] getNormals() {
        return mVertices.getFloatVec("a_normal").array();
    }

    /**
     * Sets the normal vectors of the mesh. Each normal vector is represented as
     * a packed {@code float} triplet:
     * <p>
     * <code>{ x0, y0, z0, x1, y1, z1, x2, y2, z2, ...}</code>
     * 
     * @param normals
     *            Array containing the packed normal data.
     */
    public void setNormals(float[] normals) {
        mVertices.setFloatVec("a_normal", normals);
    }

    /**
     * Get the u,v texture coordinates for the mesh. Each texture coordinate is
     * represented as a packed {@code float} pair:
     * <p>
     * <code>{ u0, v0, u1, v1, u2, v2, ...}</code>
     * 
     * @return Array with the packed texture coordinate data.
     */
    public float[] getTexCoords() {
        return mVertices.getFloatVec("a_texcoord").array();
    }

    /**
     * Sets the texture coordinates for the mesh. Each texture coordinate is
     * represented as a packed {@code float} pair:
     * <p>
     * <code>{ u0, v0, u1, v1, u2, v2, ...}</code>
     * 
     * @param texCoords
     *            Array containing the packed texture coordinate data.
     */
    public void setTexCoords(float[] texCoords)
    {
        setTexCoords(texCoords, 0);
    }

    public void setTexCoords(float [] texCoords, int index)
    {
        String key = (index > 0) ? ("a_texcoord" + index) : "a_texcoord";
        mVertices.setFloatVec(key, texCoords);
    }

    /**
     * Get the triangle vertex indices of the mesh. The indices for each
     * triangle are represented as a packed {@code char} triplet, where
     * {@code t0} is the first triangle, {@code t1} is the second, etc.:
     * <p>
     * <code>
     * { t0[0], t0[1], t0[2], t1[0], t1[1], t1[2], ...}
     * </code>
     * 
     * @return char array with the packed triangle index data.
     *
     */
    public char[] getTriangles() {
        return (mIndices != null) ? mIndices.asCharArray() : null;
    }

    /**
     * Sets the triangle vertex indices of the mesh. The indices for each
     * triangle are represented as a packed {@code int} triplet, where
     * {@code t0} is the first triangle, {@code t1} is the second, etc.:
     * <p>
     * <code>
     * { t0[0], t0[1], t0[2], t1[0], t1[1], t1[2], ...}
     * </code>
     * 
     * @param triangles
     *            Array containing the packed triangle index data.
     */
    public void setTriangles(char[] triangles)
    {
        if ((mIndices == null) && (triangles != null))
        {
            mIndices = new GVRIndexBuffer(getGVRContext(), 2, triangles.length);
            NativeMesh.setIndexBuffer(getNative(), mIndices.getNative());
        }
        else
        {
            mIndices.setShortVec(triangles);
        }
    }

    public void setTriangles(int[] triangles) {
        if ((mIndices == null) && (triangles != null))
        {
            mIndices = new GVRIndexBuffer(getGVRContext(), 4, triangles.length);
            NativeMesh.setIndexBuffer(getNative(), mIndices.getNative());
        }
        else
        {
            mIndices.setIntVec(triangles);
        }

    }

    /**
     * Get the vertex indices of the mesh. The indices for each
     * vertex to be referenced.
     * 
     * @return int array with the packed index data.
     */
    public int[] getIndices() {
        return (mIndices != null) ? mIndices.asIntArray() : null;
    }

    /**
     * Sets the vertex indices of the mesh. The indices for each
     * vertex.
     * 
     * @param indices
     *            int array containing the packed index data.
     */
    public void setIndices(int[] indices)
    {
        if (indices != null)
        {
            if (mIndices == null)
            {
                setIndexBuffer(new GVRIndexBuffer(getGVRContext(), 4, indices.length));
            }
            mIndices.setIntVec(indices);
        }
        else
        {
            mIndices = null;
            NativeMesh.setIndexBuffer(getNative(), 0L);
        }
    }

    public void setIndices(char[] indices)
    {
        if (indices != null)
        {
            if (mIndices == null)
            {
                setIndexBuffer(new GVRIndexBuffer(getGVRContext(), 2, indices.length));
            }
            mIndices.setShortVec(indices);
        }
        else
        {
            mIndices = null;
            NativeMesh.setIndexBuffer(getNative(), 0L);
        }
    }

    public void setIndices(CharBuffer indices)
    {
        if (indices != null)
        {
            if (mIndices == null)
            {
                setIndexBuffer(new GVRIndexBuffer(getGVRContext(), 2, indices.capacity() / 2));
            }
            mIndices.setShortVec(indices);
         }
        else
        {
            NativeMesh.setIndexBuffer(getNative(), 0L);
        }
    }

    /**
     * Get the array of {@code float} values associated with the vertex attribute
     * {@code key}.
     * 
     * @param key   Name of the shader attribute
     * @return Array of {@code float} values containing the vertex data for the named channel.
     */
    public float[] getFloatVec(String key)
    {
        return mVertices.getFloatArray(key);
    }

    /**
     * Bind an array of {@code int} values to the vertex attribute
     * {@code key}.
     *
     * @param key      Name of the vertex attribute
     * @param vector   Data to bind to the shader attribute.
     * @throws IllegalArgumentException if int array is wrong size
     */
    public void setIntVec(String key, int[] vector)
    {
        mVertices.setIntVec(key, vector);
    }

    /**
     * Bind an array of {@code float} values to the vertex attribute
     * {@code key}.
     *
     * @param key           Name of the vertex attribute
     * @param floatVector   Data to bind to the shader attribute.
     * @throws IllegalArgumentException if attribute name not in descriptor or float array is wrong size
     */
    public void setFloatVec(String key, float[] floatVector)
    {
        mVertices.setFloatVec(key, floatVector);
    }

    /**
     * Calculate a bounding sphere from the mesh vertices.
     * @param sphere        float[4] array to get center of sphere and radius;
     *                      sphere[0] = center.x, sphere[1] = center.y, sphere[2] = center.z, sphere[3] = radius
     */
    public void getSphereBound(float[] sphere)
    {
        mVertices.getSphereBound(sphere);
    }

    /**
     * Calculate a bounding box from the mesh vertices.
     * @param bounds        float[6] array to get corners of box;
     *                      bounds[0,1,2] = minimum X,Y,Z and bounds[3,4,6] = maximum X,Y,Z
     */
    public void getBoxBound(float[] bounds)
    {
        mVertices.getBoxBound(bounds);
    }

    /**
     * Determine if a named attribute exists in this mesh.
     * @param key Name of the shader attribute
     * @return true if attribute exists, false if not
     */
    public boolean hasAttribute(String key) {
    	return mVertices.hasAttribute(key);
    }
    
    /**
     * Constructs a {@link GVRMesh mesh} that contains this mesh.
     * 
     * <p>
     * In previous versions of GearVRF this was used with the {@link GVRPicker},
     * and the {@link GVREyePointeeHolder} which required you to pass a
     * mesh to collide against. Ray casting is computationally expensive,
     * and you generally want to limit the number of triangles to check.
     * A simple {@linkplain GVRContext#createQuad(float, float) quad} is cheap enough,
     * but with complex meshes you will probably want to cut search time by
     * registering the object's bounding box, not the whole mesh.
     * <p>
     * In newer releaes,  {@link GVRMeshCollider} automatically computea the mesh bounds
     * of the scene object it is attached to, making this function less needed.
     * @return A {@link GVRMesh} of the bounding box.
     */
    public GVRMesh getBoundingBox()
    {
        GVRMesh meshbox = new GVRMesh(getGVRContext(), "float3 a_position");
        float[] bbox = new float[6];

        getBoxBound(bbox);
        float min_x = bbox[0];
        float min_y = bbox[1];
        float min_z = bbox[2];
        float max_x = bbox[3];
        float max_y = bbox[4];
        float max_z = bbox[5];
        float[] positions = {
                min_x, min_y, min_z,
                max_x, min_y, min_z,
                min_x, max_y, min_z,
                max_x, max_y, min_z,
                min_x, min_y, max_z,
                max_x, min_y, max_z,
                min_x, max_y, max_z,
                max_x, max_y, max_z
        };
        char indices[] = {
                0, 2, 1, 1, 2, 3, 1, 3, 7, 1, 7, 5, 4, 5, 6, 5, 7, 6, 0, 6, 2, 0, 4, 6, 0, 1, 5, 0,
                5, 4, 2, 7, 3, 2, 6, 7
        };
        meshbox.setVertices(positions);
        meshbox.setTriangles(indices);
        return meshbox;
    }

    /**
     * Returns the bones of this mesh.
     *
     * @return a list of bones
     */
    public List<GVRBone> getBones() {
        return mBones;
    }

    /**
     * Sets bones of this mesh.
     *
     * @param bones a list of bones
     */
    public void setBones(List<GVRBone> bones) {
        mBones.clear();
        mBones.addAll(bones);

        NativeMesh.setBones(getNative(), GVRHybridObject.getNativePtrArray(mBones));

        // Process bones
        int boneId = -1;
        for (GVRBone bone : mBones) {
            boneId++;

            List<GVRBoneWeight> boneWeights = bone.getBoneWeights();
            for (GVRBoneWeight weight : boneWeights) {
                int vid = weight.getVertexId();
                int boneSlot = getVertexBoneData().getFreeBoneSlot(vid);
                if (boneSlot >= 0) {
                    getVertexBoneData().setVertexBoneWeight(vid, boneSlot, boneId, weight.getWeight());
                } else {
                    Log.w(TAG, "Vertex %d (total %d) has too many bones", vid, getVertices().length / 3);
                }
            }
        }
        if (getVertexBoneData() != null) {
            getVertexBoneData().normalizeWeights();
        }
    }

    /**
     * Gets the vertex bone data.
     *
     * @return the vertex bone data.
     */
    public GVRVertexBoneData getVertexBoneData() {
        return mVertexBoneData;
    }

    @Override
    public void prettyPrint(StringBuffer sb, int indent) {
        mVertices.prettyPrint(sb, indent);
        if (mIndices != null)
        {
            mIndices.prettyPrint(sb, indent);
        }
        sb.append(getBones() == null ? 0 : Integer.toString(getBones().size()));
        sb.append(" bones");
        sb.append(System.lineSeparator());

        // Bones
        List<GVRBone> bones = getBones();
        if (!bones.isEmpty()) {
            sb.append(Log.getSpaces(indent));
            sb.append("Bones:");
            sb.append(System.lineSeparator());

            for (GVRBone bone : bones) {
                bone.prettyPrint(sb, indent + 2);
            }
        }
    }

    @Override
    public String toString() {
        StringBuffer sb = new StringBuffer();
        prettyPrint(sb, 0);
        return sb.toString();
    }
}

class NativeMesh {
    static native long ctorBuffers(long vertexBuffer, long indexBuffer);

    static native void setBones(long mesh, long[] bonePtrs);

    static native void setIndexBuffer(long mesh, long ibuf);

    static native void setVertexBuffer(long mesh, long vbuf);
}
