#pragma once

#include <GEO/GEO_IOTranslator.h>
#include <UT/UT_String.h>

class GEO_PrimPoly;
class GU_Detail;
class GU_PrimVolume;

struct GEO_VoxChunk
{
    unsigned int chunk_id;
    unsigned int content_size;
    unsigned int children_chunk_size;
};

struct GEO_VoxPaletteColor
{
    union
    {
        struct
        {
            unsigned char r;
            unsigned char g;
            unsigned char b;
            unsigned char a;
        };
        unsigned char data_c[4];
        unsigned int data_u;
    };
};

struct GEO_VoxColor
{
    union
    {
        struct
        {
            float r;
            float g;
            float b;
            float a;
        };
        float data[4];
    };
};

struct GEO_VoxVoxel
{
    unsigned char x;
    unsigned char y;
    unsigned char z;
    unsigned char palette_index;
};

// reference this site https://paulbourke.net/dataformats/vox/

struct GEO_VoxSize {
	int x;
	int y;
	int z;
};

struct GEO_VoxModel {
    UT_Array<GEO_VoxVoxel> voxels;
};

struct GEO_VoxKeyValuePair {
    UT_String key;
    UT_String value;
};

struct GEO_VoxDictionary {
	UT_Array<GEO_VoxKeyValuePair> entries;
};

struct GEO_VoxNode {
    int node_id = -1;
    UT_Array<int> children;

    int transform_id = -1;
    int shape_id = -1;
    int group_id = -1;
};

struct GEO_VoxFrame {
    unsigned char rotation = 0;
    int x = 0;
    int y = 0;
    int z = 0;
    int frame_index = 0;
};

struct GEO_VoxTransform {
    int layer_id;
    GEO_VoxDictionary attributes;
    UT_Array<GEO_VoxFrame> frames;
};


struct GEO_VoxGroup {
    GEO_VoxDictionary attributes;
};

struct GEO_VoxModelReference {
    int model_id;
    GEO_VoxDictionary attributes;
};

struct GEO_VoxShape {
    GEO_VoxDictionary attributes;
    UT_Array<GEO_VoxModelReference> models;
};


struct GEO_VoxLayer {
    int layer_id;
    UT_String name;
    int8 hidden;
    GEO_VoxDictionary attributes;
};

struct GEO_VoxMaterial {
    int material_id;
    GEO_VoxDictionary attributes;
};

struct GEO_VoxObj {
    GEO_VoxDictionary renderring_attributes;
};

struct GEO_VoxCam {
    int camera_id;
    GEO_VoxDictionary attributes;
};

struct GEO_VoxNote {
    UT_Array<UT_String> color_names;
};

struct GEO_VoxIMap {
	int palette_index_association[256];
};

class GEO_Vox : public GEO_IOTranslator
{
    public:

        GEO_Vox();
        GEO_Vox(const GEO_Vox& ref);
        virtual ~GEO_Vox();

    public:

        virtual GEO_IOTranslator* duplicate() const;
        virtual const char* formatName() const;
        virtual int checkExtension(const char* name);
        virtual int checkMagicNumber(unsigned magic);
        virtual GA_Detail::IOStatus fileLoad(GEO_Detail* detail, UT_IStream& stream, bool ate_magic);
        virtual GA_Detail::IOStatus fileSave(const GEO_Detail* detail, std::ostream& stream);

    protected:

        //! Read a chunk.
        bool ReadVoxChunk(UT_IStream& stream, GEO_VoxChunk& chunk, unsigned int& bytes_read);

        //! Read a palette entry.
        bool ReadPaletteColor(UT_IStream& stream, GEO_VoxPaletteColor& palette_color, unsigned int& bytes_read);

        //! Convert palette entry from default palette value.
        void ConvertDefaultPaletteColor(unsigned int color, GEO_VoxPaletteColor& palette_color);

        //! Read a voxel entry.
        bool ReadVoxel(UT_IStream& stream, GEO_VoxVoxel& vox_voxel, unsigned int& bytes_read);

        //! Convert palette to a color.
        GEO_VoxColor ConvertPaletteColor(const GEO_VoxPaletteColor& palette_color) const;

        //! Return true if palette color corresponds to an empty voxel.
        bool IsPaletteColorEmpty(const GEO_VoxPaletteColor& palette_color) const;


        //! Read size.
        bool ReadSize(UT_IStream& stream, GEO_VoxSize& size, unsigned int& bytes_read);
        //! Read a model.
        bool ReadModel(UT_IStream& stream, GEO_VoxModel& model, unsigned int& bytes_read);
        //! Read Transform
        bool ReadTransform(UT_IStream& stream, GEO_VoxNode& node, GEO_VoxTransform& transform, unsigned int& bytes_read);
        //! Read Shape
        bool ReadShape(UT_IStream& stream, GEO_VoxNode& node, GEO_VoxShape& shape, unsigned int& bytes_read);
        //! Read Model
        bool ReadModelReference(UT_IStream& stream, GEO_VoxModelReference& modelReference, unsigned int& bytes_read);
        //! Read Group
        bool ReadGroup(UT_IStream& stream, GEO_VoxNode& node, GEO_VoxGroup& group, unsigned int& bytes_read);
        //! Read Layer
        bool ReadLayer(UT_IStream& stream, GEO_VoxLayer& layer, unsigned int& bytes_read);
        //! Read Material
        bool ReadMaterial(UT_IStream& stream, GEO_VoxMaterial& material, unsigned int& bytes_read);
        //! Read Object
        bool ReadObject(UT_IStream& stream, GEO_VoxObj& obj, unsigned int& bytes_read);
        //! Read Camera
        bool ReadCamera(UT_IStream& stream, GEO_VoxCam& cam, unsigned int& bytes_read);
        //! Read Note
        bool ReadNote(UT_IStream& stream, GEO_VoxNote& note, unsigned int& bytes_read);
        //! Read IMap
        bool ReadIMap(UT_IStream& stream, GEO_VoxIMap& imap, unsigned int& bytes_read);

        //! Read Dictionary
        bool ReadDictionary(UT_IStream& stream, GEO_VoxDictionary& dictionary, unsigned int& bytes_read);
        //! Read a string.
        bool ReadString(UT_IStream& stream, UT_String& value, unsigned int& bytes_read);
        //! Read a 32-bit integer.
        bool ReadInt(UT_IStream& stream, int& value, unsigned int& bytes_read);
        //! Get string from dictionary.
        UT_String GetDictionaryString(const GEO_VoxDictionary& dictionary, const UT_String& key) const;
        //! Traverse node hierarchy.
        bool TraverseNodes(GEO_VoxNode& node, GEO_Detail* detail, GU_PrimVolume* volume);

        template <typename T> T GetValueFromDictionary(const GEO_VoxDictionary& value, const UT_String& key, int index) const;

    protected:

        //! Magic numbers used by parser.
        static const unsigned int s_vox_magic;
        static const unsigned int s_vox_main;
        static const unsigned int s_vox_size;
        static const unsigned int s_vox_xyzi;
        static const unsigned int s_vox_rgba;
        static const unsigned int s_vox_nTRN;
        static const unsigned int s_vox_nGRP;
        static const unsigned int s_vox_nSHP;
        static const unsigned int s_vox_MATL;
        static const unsigned int s_vox_rOBJ;
        static const unsigned int s_vox_rCAM;
        static const unsigned int s_vox_LAYR;
        static const unsigned int s_vox_IMAP;
        static const unsigned int s_vox_NOTE;


        //! Palette size.
        static const unsigned int s_vox_palette_size;

        //! Supported version.
        static const unsigned int s_vox_version;

        //! Default palette data.
        static const unsigned int s_vox_default_palette[256];

    protected:

        //! Cached filename.
        UT_String m_filename;

        UT_Array<GEO_VoxPaletteColor> vox_palette;
        UT_Array<GEO_VoxSize> vox_sizes;
        UT_Array<GEO_VoxModel> vox_models;
        UT_Array<GEO_VoxTransform> vox_transforms;
        UT_Array<GEO_VoxGroup> vox_groups;
        UT_Array<GEO_VoxShape> vox_shapes;
        UT_Array<GEO_VoxNode> vox_nodes;
        UT_Array<GEO_VoxMaterial> vox_materials;

};
