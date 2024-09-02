#include "GEO_Vox.h"

#include <UT/UT_DSOVersion.h>
#include <GEO/GEO_Detail.h>
#include <GEO/GEO_PrimPoly.h>
#include <GU/GU_Detail.h>
#include <GU/GU_PrimVolume.h>
#include <UT/UT_IOTable.h>
#include <UT/UT_Assert.h>
#include <UT/UT_Endian.h>
#include <UT/UT_Algorithm.h>
#include <SYS/SYS_Math.h>

#define GEOVOX_SWAP_HOUDINI_AXIS
#define GEOVOX_VOLUME_NAME "color_lut"

#define GEOVOX_MAKE_ID(A, B, C, D) ( A ) | ( B << 8 ) | ( C << 16 ) | ( D << 24 )

const unsigned int GEO_Vox::s_vox_magic = GEOVOX_MAKE_ID('V', 'O', 'X', ' ');
const unsigned int GEO_Vox::s_vox_main = GEOVOX_MAKE_ID('M', 'A', 'I', 'N');
const unsigned int GEO_Vox::s_vox_size = GEOVOX_MAKE_ID('S', 'I', 'Z', 'E');
const unsigned int GEO_Vox::s_vox_xyzi = GEOVOX_MAKE_ID('X', 'Y', 'Z', 'I');
const unsigned int GEO_Vox::s_vox_rgba = GEOVOX_MAKE_ID('R', 'G', 'B', 'A');
const unsigned int GEO_Vox::s_vox_nTRN = GEOVOX_MAKE_ID('n', 'T', 'R', 'N');
const unsigned int GEO_Vox::s_vox_nGRP = GEOVOX_MAKE_ID('n', 'G', 'R', 'P');
const unsigned int GEO_Vox::s_vox_nSHP = GEOVOX_MAKE_ID('n', 'S', 'H', 'P');
const unsigned int GEO_Vox::s_vox_MATL = GEOVOX_MAKE_ID('M', 'A', 'T', 'L');
const unsigned int GEO_Vox::s_vox_rOBJ = GEOVOX_MAKE_ID('r', 'O', 'B', 'J');
const unsigned int GEO_Vox::s_vox_rCAM = GEOVOX_MAKE_ID('r', 'C', 'A', 'M');
const unsigned int GEO_Vox::s_vox_LAYR = GEOVOX_MAKE_ID('L', 'A', 'Y', 'R');
const unsigned int GEO_Vox::s_vox_IMAP = GEOVOX_MAKE_ID('I', 'M', 'A', 'P');
const unsigned int GEO_Vox::s_vox_NOTE = GEOVOX_MAKE_ID('N', 'O', 'T', 'E');








const unsigned int GEO_Vox::s_vox_version = 150u;
const unsigned int GEO_Vox::s_vox_palette_size = 256u;

// Taken from https://voxel.codeplex.com/wikipage?title=Sample%20Codes .
const unsigned int GEO_Vox::s_vox_default_palette[256u] =
{
    0x00000000, 0xffffffff, 0xffccffff, 0xff99ffff, 0xff66ffff, 0xff33ffff,
    0xff00ffff, 0xffffccff, 0xffccccff, 0xff99ccff, 0xff66ccff, 0xff33ccff,
    0xff00ccff, 0xffff99ff, 0xffcc99ff, 0xff9999ff, 0xff6699ff, 0xff3399ff,
    0xff0099ff, 0xffff66ff, 0xffcc66ff, 0xff9966ff, 0xff6666ff, 0xff3366ff,
    0xff0066ff, 0xffff33ff, 0xffcc33ff, 0xff9933ff, 0xff6633ff, 0xff3333ff,
    0xff0033ff, 0xffff00ff, 0xffcc00ff, 0xff9900ff, 0xff6600ff, 0xff3300ff,
    0xff0000ff, 0xffffffcc, 0xffccffcc, 0xff99ffcc, 0xff66ffcc, 0xff33ffcc,
    0xff00ffcc, 0xffffcccc, 0xffcccccc, 0xff99cccc, 0xff66cccc, 0xff33cccc,
    0xff00cccc, 0xffff99cc, 0xffcc99cc, 0xff9999cc, 0xff6699cc, 0xff3399cc,
    0xff0099cc, 0xffff66cc, 0xffcc66cc, 0xff9966cc, 0xff6666cc, 0xff3366cc,
    0xff0066cc, 0xffff33cc, 0xffcc33cc, 0xff9933cc, 0xff6633cc, 0xff3333cc,
    0xff0033cc, 0xffff00cc, 0xffcc00cc, 0xff9900cc, 0xff6600cc, 0xff3300cc,
    0xff0000cc, 0xffffff99, 0xffccff99, 0xff99ff99, 0xff66ff99, 0xff33ff99,
    0xff00ff99, 0xffffcc99, 0xffcccc99, 0xff99cc99, 0xff66cc99, 0xff33cc99,
    0xff00cc99, 0xffff9999, 0xffcc9999, 0xff999999, 0xff669999, 0xff339999,
    0xff009999, 0xffff6699, 0xffcc6699, 0xff996699, 0xff666699, 0xff336699,
    0xff006699, 0xffff3399, 0xffcc3399, 0xff993399, 0xff663399, 0xff333399,
    0xff003399, 0xffff0099, 0xffcc0099, 0xff990099, 0xff660099, 0xff330099,
    0xff000099, 0xffffff66, 0xffccff66, 0xff99ff66, 0xff66ff66, 0xff33ff66,
    0xff00ff66, 0xffffcc66, 0xffcccc66, 0xff99cc66, 0xff66cc66, 0xff33cc66,
    0xff00cc66, 0xffff9966, 0xffcc9966, 0xff999966, 0xff669966, 0xff339966,
    0xff009966, 0xffff6666, 0xffcc6666, 0xff996666, 0xff666666, 0xff336666,
    0xff006666, 0xffff3366, 0xffcc3366, 0xff993366, 0xff663366, 0xff333366,
    0xff003366, 0xffff0066, 0xffcc0066, 0xff990066, 0xff660066, 0xff330066,
    0xff000066, 0xffffff33, 0xffccff33, 0xff99ff33, 0xff66ff33, 0xff33ff33,
    0xff00ff33, 0xffffcc33, 0xffcccc33, 0xff99cc33, 0xff66cc33, 0xff33cc33,
    0xff00cc33, 0xffff9933, 0xffcc9933, 0xff999933, 0xff669933, 0xff339933,
    0xff009933, 0xffff6633, 0xffcc6633, 0xff996633, 0xff666633, 0xff336633,
    0xff006633, 0xffff3333, 0xffcc3333, 0xff993333, 0xff663333, 0xff333333,
    0xff003333, 0xffff0033, 0xffcc0033, 0xff990033, 0xff660033, 0xff330033,
    0xff000033, 0xffffff00, 0xffccff00, 0xff99ff00, 0xff66ff00, 0xff33ff00,
    0xff00ff00, 0xffffcc00, 0xffcccc00, 0xff99cc00, 0xff66cc00, 0xff33cc00,
    0xff00cc00, 0xffff9900, 0xffcc9900, 0xff999900, 0xff669900, 0xff339900,
    0xff009900, 0xffff6600, 0xffcc6600, 0xff996600, 0xff666600, 0xff336600,
    0xff006600, 0xffff3300, 0xffcc3300, 0xff993300, 0xff663300, 0xff333300,
    0xff003300, 0xffff0000, 0xffcc0000, 0xff990000, 0xff660000, 0xff330000,
    0xff0000ee, 0xff0000dd, 0xff0000bb, 0xff0000aa, 0xff000088, 0xff000077,
    0xff000055, 0xff000044, 0xff000022, 0xff000011, 0xff00ee00, 0xff00dd00,
    0xff00bb00, 0xff00aa00, 0xff008800, 0xff007700, 0xff005500, 0xff004400,
    0xff002200, 0xff001100, 0xffee0000, 0xffdd0000, 0xffbb0000, 0xffaa0000,
    0xff880000, 0xff770000, 0xff550000, 0xff440000, 0xff220000, 0xff110000,
    0xffeeeeee, 0xffdddddd, 0xffbbbbbb, 0xffaaaaaa, 0xff888888, 0xff777777,
    0xff555555, 0xff444444, 0xff222222, 0xff111111
};


void
newGeometryIO(void* parm)
{
    GU_Detail::registerIOTranslator(new GEO_Vox());

    UT_ExtensionList* extension = UTgetGeoExtensions();
    if(!extension->findExtension("vox"))
    {
        extension->addExtension("vox");
    }
}


GEO_Vox::GEO_Vox() :
    m_filename(UT_String::ALWAYS_DEEP)
{

}


GEO_Vox::GEO_Vox(const GEO_Vox& ref) :
    m_filename(UT_String::ALWAYS_DEEP, ref.m_filename)
{

}


GEO_Vox::~GEO_Vox()
{

}


GEO_IOTranslator*
GEO_Vox::duplicate() const
{
    return new GEO_Vox(*this);
}


const char*
GEO_Vox::formatName() const
{
    return "Vox";
}


int
GEO_Vox::checkExtension(const char* name)
{
    UT_String str_name(UT_String::ALWAYS_DEEP, name);
    const char* ext = str_name.fileExtension();

    if(!ext)
    {
        return 0;
    }

    if(!strcasecmp(ext, ".vox"))
    {
        m_filename = str_name;
        return 1;
    }

    return 0;
}


int
GEO_Vox::checkMagicNumber(unsigned magic)
{
    return GEO_Vox::s_vox_magic == magic;
}


GA_Detail::IOStatus
GEO_Vox::fileLoad(GEO_Detail* detail, UT_IStream& stream, bool ate_magic)
{
    unsigned int vox_child_bytes_read = 0u;

    GU_Detail* gu_detail = dynamic_cast<GU_Detail*>(detail);
    UT_ASSERT(gu_detail);

    // Not sure what's going on since H18, seems to be consuming magic, but not flaging it as such.

    if(!ate_magic)
    {
        unsigned int vox_magic_number = 0;
        if(stream.bread(&vox_magic_number) != 1)
        {
            detail->clearAndDestroy();
            return GA_Detail::IOStatus(false);
        }

        UTswap_int32(vox_magic_number, vox_magic_number);

        if(!checkMagicNumber(vox_magic_number))
        {
            detail->clearAndDestroy();
            return GA_Detail::IOStatus(false);
        }
    }

    unsigned int vox_version = 0;
    if(stream.bread(&vox_version) != 1)
    {
        detail->clearAndDestroy();
        return GA_Detail::IOStatus(false);
    }

    UTswap_int32(vox_version, vox_version);

    if(GEO_Vox::s_vox_version != vox_version)
    {
        detail->clearAndDestroy();
        return GA_Detail::IOStatus(false);
    }

    GEO_VoxChunk vox_chunk_main;
    if(!ReadVoxChunk(stream, vox_chunk_main, vox_child_bytes_read))
    {
        detail->clearAndDestroy();
        return GA_Detail::IOStatus(false);
    }

    if(GEO_Vox::s_vox_main != vox_chunk_main.chunk_id)
    {
        detail->clearAndDestroy();
        return GA_Detail::IOStatus(false);
    }

    // We skip the content of main chunk.
    if(!stream.seekg(vox_chunk_main.content_size, UT_IStream::UT_SEEK_CUR))
    {
        detail->clearAndDestroy();
        return GA_Detail::IOStatus(false);
    }

    // Variables to read voxel data into.
    unsigned int vox_size_x = 0u;
    unsigned int vox_size_y = 0u;
    unsigned int vox_size_z = 0u;

    // Reset bytes read.
    vox_child_bytes_read = 0u;

    // We start reading chunks specified in the main chunk.
    while(vox_child_bytes_read != vox_chunk_main.children_chunk_size)
    {
        GEO_VoxChunk vox_chunk_child;
        if(!ReadVoxChunk(stream, vox_chunk_child, vox_child_bytes_read))
        {
            detail->clearAndDestroy();
            return GA_Detail::IOStatus(false);
        }

        if(GEO_Vox::s_vox_size == vox_chunk_child.chunk_id)
        {
            GEO_VoxSize size;
            if(!ReadSize(stream, size, vox_child_bytes_read))
			{
				detail->clearAndDestroy();
				return GA_Detail::IOStatus(false);
			}
            vox_sizes.append(size);
        }
        else if(GEO_Vox::s_vox_xyzi == vox_chunk_child.chunk_id)
        {
            GEO_VoxModel model;
            if(!ReadModel(stream, model, vox_child_bytes_read))
			{
				detail->clearAndDestroy();
				return GA_Detail::IOStatus(false);
			}
            vox_models.append(model);
        }
        else if(GEO_Vox::s_vox_rgba == vox_chunk_child.chunk_id)
        {
            vox_palette.setSize(GEO_Vox::s_vox_palette_size);
            for(unsigned int idx = 0; idx < GEO_Vox::s_vox_palette_size; ++idx)
            {
                GEO_VoxPaletteColor vox_palette_color;
                if(!ReadPaletteColor(stream, vox_palette_color, vox_child_bytes_read))
                {
                    detail->clearAndDestroy();
                    return GA_Detail::IOStatus(false);
                }

                vox_palette(idx) = vox_palette_color;
            }
        }
        else if (GEO_Vox::s_vox_nTRN == vox_chunk_child.chunk_id)
        {
            GEO_VoxNode node;
			GEO_VoxTransform transform;
			if (!ReadTransform(stream, node, transform, vox_child_bytes_read))
			{
				detail->clearAndDestroy();
				return GA_Detail::IOStatus(false);
			}
            node.transform_id = vox_transforms.size();
            vox_nodes.append(node);
            vox_transforms.append(transform);
		}
		else if (GEO_Vox::s_vox_nGRP == vox_chunk_child.chunk_id)
		{
			GEO_VoxNode node;
			GEO_VoxGroup group;
			if (!ReadGroup(stream, node, group, vox_child_bytes_read))
			{
				detail->clearAndDestroy();
				return GA_Detail::IOStatus(false);
			}
            node.group_id = vox_groups.size();
            vox_nodes.append(node);
            vox_groups.append(group);

		}
		else if (GEO_Vox::s_vox_nSHP == vox_chunk_child.chunk_id)
		{
			GEO_VoxNode node;
			GEO_VoxShape shape;
			if (!ReadShape(stream, node, shape, vox_child_bytes_read))
			{
				detail->clearAndDestroy();
				return GA_Detail::IOStatus(false);
			}
            node.shape_id = vox_shapes.size();
			vox_nodes.append(node);
			vox_shapes.append(shape);
		}
		else if (GEO_Vox::s_vox_MATL == vox_chunk_child.chunk_id)
		{
			GEO_VoxMaterial material;
			if (!ReadMaterial(stream, material, vox_child_bytes_read))
			{
				detail->clearAndDestroy();
				return GA_Detail::IOStatus(false);
			}
		}
		else if (GEO_Vox::s_vox_rOBJ == vox_chunk_child.chunk_id)
		{
            GEO_VoxObj renderObject;
			if (!ReadObject(stream, renderObject, vox_child_bytes_read))
			{
				detail->clearAndDestroy();
				return GA_Detail::IOStatus(false);
			}
		}
		else if (GEO_Vox::s_vox_rCAM == vox_chunk_child.chunk_id)
		{
			GEO_VoxCam renderCamera;
			if (!ReadCamera(stream, renderCamera, vox_child_bytes_read))
			{
				detail->clearAndDestroy();
				return GA_Detail::IOStatus(false);
			}
		}
		else if (GEO_Vox::s_vox_LAYR == vox_chunk_child.chunk_id)
		{
			GEO_VoxLayer layer;
			if (!ReadLayer(stream, layer, vox_child_bytes_read))
			{
				detail->clearAndDestroy();
				return GA_Detail::IOStatus(false);
			}
		}
		else if (GEO_Vox::s_vox_IMAP == vox_chunk_child.chunk_id)
		{
        }

        else
        {
            // We don't know this chunk, skip content in addition to skipping children.
            if(!stream.seekg(vox_chunk_child.content_size, UT_IStream::UT_SEEK_CUR))
            {
                detail->clearAndDestroy();
                return GA_Detail::IOStatus(false);
            }

            vox_child_bytes_read += vox_chunk_child.content_size;
        }

        // Skip children.
        if(!stream.seekg(vox_chunk_child.children_chunk_size, UT_IStream::UT_SEEK_CUR))
        {
            detail->clearAndDestroy();
            return GA_Detail::IOStatus(false);
        }

        if(vox_chunk_child.children_chunk_size > 0u)
        {
            vox_child_bytes_read += vox_chunk_child.children_chunk_size;
        }
    }

    // If there was no palette, use default.
    if(0 == vox_palette.size())
    {
        vox_palette.setSize(GEO_Vox::s_vox_palette_size);
        for(unsigned int idx = 0; idx < GEO_Vox::s_vox_palette_size; ++idx)
        {
            GEO_VoxPaletteColor vox_palette_color;
            ConvertDefaultPaletteColor(GEO_Vox::s_vox_default_palette[idx], vox_palette_color);
            vox_palette(idx) = vox_palette_color;
        }
    }

    if(!TraverseNodes(vox_nodes(0), gu_detail, nullptr))
	{
		detail->clearAndDestroy();
		return GA_Detail::IOStatus(false);
	}

    return GA_Detail::IOStatus(true);
}


GA_Detail::IOStatus
GEO_Vox::fileSave(const GEO_Detail* detail, std::ostream& stream)
{
    return GA_Detail::IOStatus(false);
}


bool
GEO_Vox::ReadVoxChunk(UT_IStream& stream, GEO_VoxChunk& chunk, unsigned int& bytes_read)
{
    if(stream.bread(&chunk.chunk_id) != 1)
    {
        return false;
    }

    UTswap_int32(chunk.chunk_id, chunk.chunk_id);
    bytes_read += sizeof(unsigned int);

    if(stream.bread(&chunk.content_size) != 1)
    {
        return false;
    }

    UTswap_int32(chunk.content_size, chunk.content_size);
    bytes_read += sizeof(unsigned int);

    if(stream.bread(&chunk.children_chunk_size) != 1)
    {
        return false;
    }

    UTswap_int32(chunk.children_chunk_size, chunk.children_chunk_size);
    bytes_read += sizeof(unsigned int);

    return true;
}


bool
GEO_Vox::ReadPaletteColor(UT_IStream& stream, GEO_VoxPaletteColor& palette_color, unsigned int& bytes_read)
{
    if(stream.bread(&palette_color.r) != 1)
    {
        return false;
    }

    bytes_read += sizeof(unsigned char);

    if(stream.bread(&palette_color.g) != 1)
    {
        return false;
    }

    bytes_read += sizeof(unsigned char);

    if(stream.bread(&palette_color.b) != 1)
    {
        return false;
    }

    bytes_read += sizeof(unsigned char);

    if(stream.bread(&palette_color.a) != 1)
    {
        return false;
    }

    bytes_read += sizeof(unsigned char);

    return true;
}


void
GEO_Vox::ConvertDefaultPaletteColor(unsigned int color, GEO_VoxPaletteColor& palette_color)
{
    palette_color.data_u = color;
}


GEO_VoxColor
GEO_Vox::ConvertPaletteColor(const GEO_VoxPaletteColor& palette_color) const
{
    GEO_VoxColor color;

    color.r = SYSclamp(palette_color.r, 0, 255) / 255.0f;
    color.g = SYSclamp(palette_color.g, 0, 255) / 255.0f;
    color.b = SYSclamp(palette_color.b, 0, 255) / 255.0f;
    color.a = SYSclamp(palette_color.a, 0, 255) / 255.0f;

    return color;
}


bool
GEO_Vox::IsPaletteColorEmpty(const GEO_VoxPaletteColor& palette_color) const
{
    return 0x00000000 == palette_color.data_u;
}


bool
GEO_Vox::ReadVoxel(UT_IStream& stream, GEO_VoxVoxel& vox_voxel, unsigned int& bytes_read)
{
    if(stream.bread(&vox_voxel.x) != 1)
    {
        return false;
    }

    bytes_read += sizeof(unsigned char);

    if(stream.bread(&vox_voxel.y) != 1)
    {
        return false;
    }

    bytes_read += sizeof(unsigned char);

    if(stream.bread(&vox_voxel.z) != 1)
    {
        return false;
    }

    bytes_read += sizeof(unsigned char);

    if(stream.bread(&vox_voxel.palette_index) != 1)
    {
        return false;
    }

    bytes_read += sizeof(unsigned char);

    return true;
}


//! Read size.
bool GEO_Vox::ReadSize(UT_IStream& stream, GEO_VoxSize& size, unsigned int& bytes_read)
{
    if(!ReadInt(stream, size.x, bytes_read))
	{
		return false;
	}

	if(!ReadInt(stream, size.y, bytes_read))
	{
		return false;
	}

	if(!ReadInt(stream, size.z, bytes_read))
	{
		return false;
	}

	return true;

}
//! Read a model.
bool GEO_Vox::ReadModel(UT_IStream& stream, GEO_VoxModel& model, unsigned int& bytes_read)
{
	int num_of_voxels;
	if(!ReadInt(stream, num_of_voxels, bytes_read))
	{
		return false;
	}
	for (int idx = 0; idx < num_of_voxels; ++idx)
	{
		GEO_VoxVoxel voxel;
		if(!ReadVoxel(stream, voxel, bytes_read))
		{
			return false;
		}
		model.voxels.append(voxel);
	}
	return true;
}


//! Read Transform
bool GEO_Vox::ReadTransform(UT_IStream& stream, GEO_VoxNode& node, GEO_VoxTransform& transform, unsigned int& bytes_read)
{
    if(!ReadInt(stream, node.node_id, bytes_read))
    {
        return false;
	}

    if(!ReadDictionary(stream, transform.attributes, bytes_read))
	{
		return false;
    }
    int child_node_id = -1;
	if(!ReadInt(stream, child_node_id, bytes_read))
        {
		return false;
	}
    node.children.append(child_node_id);

    int reserved;
    if(!ReadInt(stream, reserved, bytes_read))
	{
		return false;
	}
    
    if (!ReadInt(stream, transform.layer_id, bytes_read))
    {
        return false;
    }
    int num_of_frames;
    if (!ReadInt(stream, num_of_frames, bytes_read))
	{
		return false;
	}
    for (int idx = 0; idx < num_of_frames; ++idx)
    {
        GEO_VoxFrame frame;
        GEO_VoxDictionary frameAttributes;
        if (!ReadDictionary(stream, frameAttributes, bytes_read)) {
            return false;
        }

        frame.x = GetValueFromDictionary<int>(frameAttributes, "_t", 0);
        frame.y = GetValueFromDictionary<int>(frameAttributes, "_t", 1);
        frame.z = GetValueFromDictionary<int>(frameAttributes, "_t", 2);
        frame.rotation = GetValueFromDictionary<unsigned char>(frameAttributes, "_r", 0);

        transform.frames.append(frame);
    }
	return true;

}

//! Read Shape
bool GEO_Vox::ReadShape(UT_IStream& stream, GEO_VoxNode& node, GEO_VoxShape& shape, unsigned int& bytes_read)
{
    if(!ReadInt(stream, node.node_id, bytes_read))
	{
		return false;
	}
    
	if(!ReadDictionary(stream, shape.attributes, bytes_read))
	{
		return false;
	}
    int num_of_models;
	if(!ReadInt(stream, num_of_models, bytes_read))
	{
		return false;
	}
    for (int idx = 0; idx < num_of_models; ++idx)
    {
        GEO_VoxModelReference model_reference;
        if(!ReadModelReference(stream, model_reference, bytes_read))
		{
			return false;
		}
        shape.models.append(model_reference);
    }
    return true;
}
//! Read ModelReference
bool GEO_Vox::ReadModelReference(UT_IStream& stream, GEO_VoxModelReference& modelReference, unsigned int& bytes_read)
{
    if(!ReadInt(stream, modelReference.model_id, bytes_read))
	{
		return false;
	}
    if(!ReadDictionary(stream, modelReference.attributes, bytes_read))
    {
        return false;
	}
    return true;
}

//! Read Group
bool GEO_Vox::ReadGroup(UT_IStream& stream, GEO_VoxNode& node, GEO_VoxGroup& group, unsigned int& bytes_read) {
	if(!ReadInt(stream, node.node_id, bytes_read))
	{
		return false;
	}
    
    if(!ReadDictionary(stream, group.attributes, bytes_read)) {
        return false;
	}

    int num_children = 0;
    if (!ReadInt(stream, num_children, bytes_read))
    {
        return false;
    }
    for(int idx = 0; idx < num_children; ++idx)
	{
        int child = -1;
        if (!ReadInt(stream, child, bytes_read))
        {
            return false;
        }
        node.children.append(child);
	}
    return true;
}


//! Read Layer
bool GEO_Vox::ReadLayer(UT_IStream& stream, GEO_VoxLayer& layer, unsigned int& bytes_read)
{
    if(!ReadInt(stream, layer.layer_id, bytes_read))
	{
		return false;
	}
	
	if(!ReadDictionary(stream, layer.attributes, bytes_read))
	{
		return false;
	}
    int reserved;
    if(!ReadInt(stream, reserved, bytes_read))
    {
        return false;
	}
	return true;
}
//! Read Material
bool GEO_Vox::ReadMaterial(UT_IStream& stream, GEO_VoxMaterial& material, unsigned int& bytes_read)
{
    if (!ReadInt(stream, material.material_id, bytes_read))
    {
        return false;
    }

    if(!ReadDictionary(stream, material.attributes, bytes_read))
	{
		return false;
	}
	return true;
}

//! Read Object
bool GEO_Vox::ReadObject(UT_IStream& stream, GEO_VoxObj& obj, unsigned int& bytes_read)
{
    if(!ReadDictionary(stream, obj.renderring_attributes, bytes_read))
    {
        return false;
	}
    return true;
}
//! Read Camera
bool GEO_Vox::ReadCamera(UT_IStream& stream, GEO_VoxCam& cam, unsigned int& bytes_read)
{
    if (!ReadInt(stream, cam.camera_id, bytes_read))
    {
        return false;
    }

    if(!ReadDictionary(stream, cam.attributes, bytes_read))
	{
		return false;
	}
	return true;
}

//! Read Note
bool GEO_Vox::ReadNote(UT_IStream& stream, GEO_VoxNote& note, unsigned int& bytes_read) 
{
    int num_names;
    if(!ReadInt(stream, num_names, bytes_read))
        {
		return false;
	}
    for(int idx = 0; idx < num_names; ++idx)
	{
		UT_String color_name;
		if(!ReadString(stream, color_name, bytes_read))
		{
			return false;
		}
		note.color_names.append(color_name);
	}

}
//! Read IMap
bool GEO_Vox::ReadIMap(UT_IStream& stream, GEO_VoxIMap& imap, unsigned int& bytes_read) 
{
    // not implemeted
    return true;
}




//! Read Dictionary
bool GEO_Vox::ReadDictionary(UT_IStream& stream, GEO_VoxDictionary& dictionary, unsigned int& bytes_read) {
    int num_entries = 0;
	if(!ReadInt(stream, num_entries, bytes_read))
	{
		return false;
	}
	for(int idx = 0; idx < num_entries; ++idx)
	{
        GEO_VoxKeyValuePair entry;
		if(!ReadString(stream, entry.key, bytes_read))
		{
			return false;
		}
		if(!ReadString(stream, entry.value, bytes_read))
		{
			return false;
		}
		dictionary.entries.append(entry);
	}
	return true;
}


//! Read a string.
bool GEO_Vox::ReadString(UT_IStream& stream, UT_String& value, unsigned int& bytes_read) {
    int length = 0;
	if(!ReadInt(stream, length, bytes_read))
	{
		return false;
	}
	if(length < 0)
	{
		return false;
	}
	char* buffer = new char[length+1];
	if(stream.bread(buffer, length) != length)
	{
		delete[] buffer;
		return false;
	}
    buffer[length] = '\0';

    UT_String newStr(UT_String::ALWAYS_DEEP, buffer);

	value = newStr;
	delete[] buffer;
	bytes_read += length;
	return true;
}


//! Read a 32-bit integer.
bool GEO_Vox::ReadInt(UT_IStream& stream, int& value, unsigned int& bytes_read) {
    if(stream.bread(&value) != 1)
	{
		return false;
	}
	UTswap_int32(value, value);
	bytes_read += sizeof(int);
	return true;
}



//! Get string from dictionary.
UT_String GEO_Vox::GetDictionaryString(const GEO_VoxDictionary& dictionary, const UT_String& key) const
{
    UT_String value;
	for(int idx = 0; idx < dictionary.entries.size(); ++idx)
	{
		if(key == dictionary.entries(idx).key)
		{
			value = dictionary.entries(idx).value;
			break;
		}
	}
	return value;
}

template <typename T> T GEO_Vox::GetValueFromDictionary(const GEO_VoxDictionary& value, const UT_String& key, int index) const
{
    T result;
	UT_String valueStr = GetDictionaryString(value, key);
	if(valueStr.length() > 0)
	{
		UT_StringArray values;
		valueStr.tokenize(values, " ");
		if(values.size() > index)
		{
            if (std::is_same<T, int>::value)
			{
				result = values(index).toInt();
			}
			else if (std::is_same<T, float>::value)
			{
				result = values(index).toFloat();
            }
            else {
                result = values(index).toInt();
            }
		}
	}
	return result;


}




//! Traverse node hierarchy.
bool GEO_Vox::TraverseNodes(GEO_VoxNode& node, GEO_Detail* detail, GU_PrimVolume* volume)
{
    if (node.transform_id >= 0 && node.transform_id < vox_transforms.size())
    {
        GEO_VoxTransform& transform = vox_transforms(node.transform_id);
        volume = (GU_PrimVolume*)GU_PrimVolume::build((GU_Detail*)detail);
        int frameIdx = 0;
		if(frameIdx < transform.frames.size())
		{

			GEO_VoxFrame& frame = transform.frames(frameIdx);
            UT_Matrix3 matrix;
            matrix.identity();

            volume->setTransform(matrix);

#ifdef GEOVOX_SWAP_HOUDINI_AXIS
            UT_Vector3 vec(frame.x, frame.z, frame.y);
#else
            UT_Vector3 vec(frame.x, frame.y, frame.z);
#endif
            volume->setPos3(vec);
		}
        UT_String name = GetDictionaryString(transform.attributes, "_name");
        detail->addStringTuple(GA_ATTRIB_PRIMITIVE, "group_name", 1);
        GA_RWHandleS group_name_attrib(detail->findPrimitiveAttribute("group_name"));
        group_name_attrib.set(volume->getMapOffset(), name);
    }
    else if (node.group_id >= 0) {
        GEO_VoxGroup& group = vox_groups(node.group_id);
    }
    else if (node.shape_id >= 0 && node.shape_id < vox_shapes.size()) {


        GEO_VoxShape& shape = vox_shapes(node.shape_id);
        for (int idx = 0; idx < shape.models.size(); ++idx)
        {
            GEO_VoxModelReference& model_reference = shape.models(idx);
            if (model_reference.model_id >= 0 && model_reference.model_id < vox_models.size() && model_reference.model_id < vox_sizes.size())
            {
                GEO_VoxSize& size = vox_sizes(model_reference.model_id);
                UT_Matrix3 matrix = volume->getTransform();
#ifdef GEOVOX_SWAP_HOUDINI_AXIS
                matrix.scale(size.x * 0.5f, size.z * 0.5f, size.y * 0.5f);
#else
                matrix.scale(size.x * 0.5f, size.y * 0.5f, size.z * 0.5f);
#endif
                volume->setTransform(matrix);

                /*
                detail->addStringTuple(GA_ATTRIB_PRIMITIVE, "name", 1);
                GA_RWHandleS name_attrib(detail->findPrimitiveAttribute("name"));
                name_attrib.set(volume->getMapOffset(), GEOVOX_VOLUME_NAME);
                */

                UT_VoxelArrayWriteHandleF handle = volume->getVoxelWriteHandle();

#ifdef GEOVOX_SWAP_HOUDINI_AXIS
                handle->size(size.x, size.z, size.y);
#else
                handle->size(size.x, size.y, size.z);
#endif

                GEO_VoxModel& model = vox_models(model_reference.model_id);
                for (unsigned int idx_vox = 0, vox_entries = model.voxels.entries(); idx_vox < vox_entries; ++idx_vox)
                {
                    GEO_VoxVoxel vox_voxel = model.voxels(idx_vox);
                    const GEO_VoxPaletteColor& vox_palette_color = vox_palette(vox_voxel.palette_index);

                    if (!IsPaletteColorEmpty(vox_palette_color))
                    {
#ifdef GEOVOX_SWAP_HOUDINI_AXIS
                        handle->setValue(vox_voxel.x, vox_voxel.z, vox_voxel.y, (float)vox_voxel.palette_index);
#else
                        handle->setValue(vox_voxel.x, vox_voxel.y, vox_voxel.z, (float)vox_voxel.palette_index);
#endif
                    }
                }
            }
        }
    }

    for(int idx = 0; idx < node.children.size(); ++idx)
	{
        int child_id = node.children(idx);
		if(child_id < 0 || child_id >= vox_nodes.size())
		{
			return true;
		}

        GEO_VoxNode& child = vox_nodes(child_id);
		if(!TraverseNodes(child, detail, volume))
		{
			return false;
		}
	}
	return true;
}