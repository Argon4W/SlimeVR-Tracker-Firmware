#include "esp_fast_text_engine.h"
#include "esp_fast_text_engine_common.h"

void private_bake_atlas_from_1bpp(
	const	esp_fast_text_engine_instance_t*	context,
			esp_fast_text_engine_atlas_slot_t*	atlas_slot,
			uint16_t							codepoint
) {
	// Get the properties, configuration, and font of the text engine instance.
	const esp_fast_text_engine_instance_properties_t*	properties	= context		->properties;
	const esp_fast_text_engine_font_t*					font		= &properties	->font;

	// Extract necessary parameters for glyph baking.
	const uint32_t atlas_glyph_size_x	= properties->atlas_glyph_size_x;
	const uint32_t atlas_glyph_size_y	= properties->atlas_glyph_size_y;
	const uint32_t font_size_multiplier	= properties->font_size_multiplier;

	// Get the lookup index of the codepoint to bake.
	const uint32_t block = (codepoint >> 8U) & 0xFFU;
	const uint32_t index = (codepoint >> 0U) & 0xFFU;

	// Get the glyph data and the glyph info of the codepoint to bake.
	const uint8_t*		data = font->glyph_data;
	const glyph_info_t	info = font->glyph_table[block][index];

	// Get the properties of the glyph to bake.
	const uint32_t offset = info.offset;
	const uint32_t size_x = info.size_x;

	// Calculate bytes in a line of 1bpp glyph data.
	const uint32_t font_size_x_byte	= (size_x + 7U) / 8U;

	// Get the buffer pointer to bake the glyph into.
	uint16_t* buffer = atlas_slot->buffer;

	// Update the slot info.
	atlas_slot->codepoint	= codepoint;
	atlas_slot->size_x		= size_x * font_size_multiplier;

	// Bake bitmask for all pixels of the glyph.
	for		(uint32_t position_y = 0U; position_y < atlas_glyph_size_y; position_y ++) {
		for	(uint32_t position_x = 0U; position_x < atlas_glyph_size_x; position_x ++) {
			// Map the scaled bitmask pixel coordinate backed to unscaled 1bpp pixel coordinate.
			const uint32_t position_x_1bpp = position_x / font_size_multiplier;
			const uint32_t position_y_1bpp = position_y / font_size_multiplier;

			// All fonts have the same height, but they have different widths.
			if (position_x_1bpp >= size_x) {
				continue;
			}

			// Get the byte coordinate from the X of the unscaled 1bpp pixel coordinate.
			const uint32_t position_x_bits = position_x_1bpp % 8U;
			const uint32_t position_x_byte = position_x_1bpp / 8U;

			// Get the byte containing the 1bpp data of the unscaled pixel.
			const uint8_t pixel_byte = data[
				/* offset	= */ offset +
				/* index_y	= */ position_y_1bpp * font_size_x_byte +
				/* index_x	= */ position_x_byte
			];

			// Get the final bitmask value from the pixel byte.
			const uint16_t bitmask = (pixel_byte & (1U << position_x_bits)) ? 0xFFFFU : 0x0000U;

			// Write the bitmask value to the atlas.
			buffer[
				/* index_y = */ position_y * atlas_glyph_size_x +
				/* index_x = */ position_x
			] = bitmask;
		}
	}
}