#ifndef __MKSN_OBJVIEW_MODEL_IQM__
#define __MKSN_OBJVIEW_MODEL_IQM__

struct __iqm_header
{
    char magic[16]; // the string "INTERQUAKEMODEL\0", 0 terminated
    uint version; // must be version 2
    uint filesize;
    uint flags;
    uint num_text, ofs_text;
    uint num_meshes, ofs_meshes;
    uint num_vertexarrays, num_vertexes, ofs_vertexarrays;
    uint num_triangles, ofs_triangles, ofs_adjacency;
    uint num_joints, ofs_joints;
    uint num_poses, ofs_poses;
    uint num_anims, ofs_anims;
    uint num_frames, num_framechannels, ofs_frames, ofs_bounds;
    uint num_comment, ofs_comment;
    uint num_extensions, ofs_extensions; // these are stored as a linked list, not as a contiguous array
};

struct __iqm_mesh
{
    uint name;     // unique name for the mesh, if desired
    uint material; // set to a name of a non-unique material or texture
    uint first_vertex, num_vertexes;
    uint first_triangle, num_triangles;
};

// all vertex array entries must ordered as defined below, if present
// i.e. position comes before normal comes before ... comes before custom
// where a format and size is given, this means models intended for portable use should use these
// an IQM implementation is not required to honor any other format/size than those recommended
// however, it may support other format/size combinations for these types if it desires
enum // vertex array type
{
  IQM_VAT_POSITION     = 0,  // float, 3
  IQM_VAT_TEXCOORD     = 1,  // float, 2
  IQM_VAT_NORMAL       = 2,  // float, 3
  IQM_VAT_TANGENT      = 3,  // float, 4
  IQM_VAT_BLENDINDEXES = 4,  // ubyte, 4
  IQM_VAT_BLENDWEIGHTS = 5,  // ubyte, 4
  IQM_VAT_COLOR        = 6,  // ubyte, 4

    // all values up to CUSTOM are reserved for future use
    // any value >= CUSTOM is interpreted as CUSTOM type
    // the value then defines an offset into the string table, where offset = value - CUSTOM
    // this must be a valid string naming the type
  IQM_VAT_CUSTOM       = 0x10
};

enum // vertex array format
{
  IQM_VAF_BYTE   = 0,
  IQM_VAF_UBYTE  = 1,
  IQM_VAF_SHORT  = 2,
  IQM_VAF_USHORT = 3,
  IQM_VAF_INT    = 4,
  IQM_VAF_UINT   = 5,
  IQM_VAF_HALF   = 6,
  IQM_VAF_FLOAT  = 7,
  IQM_VAF_DOUBLE = 8,
};

struct __iqm_vertexarray
{
  uint type;   // type or custom name
  uint flags;
  uint format; // component format
  uint size;   // number of components
  uint offset; // offset to array of tightly packed components, with num_vertexes * size total entries
  // offset must be aligned to max(sizeof(format), 4)
};

struct __iqm_triangle
{
  uint vertex[3];
};

struct adjacency
{
  uint triangle[3];
};
 
struct __iqm_joint
{
  uint name;
  int parent; // parent < 0 means this is a root bone
  float translate[3], rotate[4], scale[3]; 
  // translate is translation <Tx, Ty, Tz>, and rotate is quaternion rotation <Qx, Qy, Qz, Qw>
  // rotation is in relative/parent local space
  // scale is pre-scaling <Sx, Sy, Sz>
  // output = (input*scale)*rotation + translation
};

struct __iqm_pose
{
  int parent; // parent < 0 means this is a root bone
  uint channelmask; // mask of which 10 channels are present for this joint pose
  float channeloffset[10], channelscale[10]; 
  // channels 0..2 are translation <Tx, Ty, Tz> and channels 3..6 are quaternion rotation <Qx, Qy, Qz, Qw>
  // rotation is in relative/parent local space
  // channels 7..9 are scale <Sx, Sy, Sz>
  // output = (input*scale)*rotation + translation
};

ushort frames[]; // frames is a big unsigned short array where each group of framechannels components is one frame

struct __iqm_anim
{
  uint name;
  uint first_frame, num_frames; 
  float framerate;
  uint flags;
};

enum
{
    ANIM_LOOP = 1<<0
};

struct __iqm_bounds
{
    float bbmins[3], bbmaxs[3]; // the minimum and maximum coordinates of the bounding box for this animation frame
    float xyradius, radius; // the circular radius in the X-Y plane, as well as the spherical radius
};

char text[]; // big array of all strings, each individual string being 0 terminated, with the first string always being the empty string "" (i.e. text[0] == 0)
char comment[];

struct __iqm_extension
{
    uint name;
    uint num_data, ofs_data;
    uint ofs_extensions; // pointer to next extension
};

// vertex data is not really interleaved, but this just gives examples of standard types of the data arrays
struct __iqm_vertex
{
    float position[3], texcoord[2], normal[3], tangent[4];
    uchar blendindices[4], blendweights[4], color[4];
};

#endif
