#ifndef ESP_FAST_TEXT_ENGINE_COMMON_H
#define ESP_FAST_TEXT_ENGINE_COMMON_H

#include "stdint.h"
#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// The log tag of esp_fast_text_engine.
static const char* ESP_FAST_TEXT_ENGINE_TAG = "esp_fast_text_engine";

/**
 * @brief					Private function of creating a new LRU atlas.
 * @param atlas_ret			The handle to receive created LRU atlas.
 * @param atlas_name		The name of the atlas, used in debug logging.
 * @param atlas_slot_count	The count of atlas slot in the atlas.
 * @param atlas_glyph_size	The count of pixels of an atlas slot.
 * @param atlas_lru_flags	The buffer flags when allocating heap pixel buffers.
 * @param atlas_flags		The buffer flags when allocating heap LRU linked list.
 * @return					The status of the creation.
 */
esp_err_t private_new_lru_atlas(
			esp_fast_text_engine_lru_atlas_t**	atlas_ret,
	const	char*								atlas_name,
			uint32_t							atlas_slot_count,
			uint32_t							atlas_glyph_size,
			uint32_t							atlas_lru_flags,
			uint32_t							atlas_flags
);

/**
 * @brief			Private function of releasing a given LRU atlas.
 * @param atlas_in	The LRU atlas handle to be released.
 * @return			The status of the releasing.
 */
esp_err_t private_del_lru_atlas(esp_fast_text_engine_lru_atlas_t* atlas_in);

/**
 * @brief				Private function of baking a glyph of given codepoint in the tightly-packed 1bpp LSbit first font
 *						glyph data to a given RGB565 bitmask pixel buffer.
 * @param context		The text engine instance to bake the glyph.
 * @param codepoint		The codepoint of the glyph to bake into the atlas.
 * @param pixel_buffer	The pixel buffer to bake the glyph into.
 * @param glyph_size_x	The handle to receive the size of the X axis in pixels of the baked bitmask of the glyph.
 * @return				The baking result.
 */
void private_bake_atlas_from_1bpp(
	const	esp_fast_text_engine_instance_t*	context,
			uint16_t							codepoint,
			uint16_t*							pixel_buffer,
			uint32_t*							glyph_size_x
);

/**
 * @brief		Private function of moving a given LRU linked node to the head of its LRU atlas linked list.
 * @param atlas	The LRU atlas of the linked node.
 * @param node	The node to be moved to the head of its linked list.
 */
void private_move_lru_node_to_head(
	esp_fast_text_engine_lru_atlas_t*	atlas,
	esp_fast_text_engine_lru_node_t*	node
);

/**
 * @brief		Private function of moving a given LRU linked node to the tail of its LRU atlas linked list.
 * @param atlas	The LRU atlas of the linked node.
 * @param node	The node to be moved to the tail of its linked list.
 */
void private_move_lru_node_to_tail(
	esp_fast_text_engine_lru_atlas_t*	atlas,
	esp_fast_text_engine_lru_node_t*	node
);

/**
 * @brief				Private function of looking up a glyph in the cache of the given text engine or load it to the cache
 *						if it is not in the cache.
 * @param context		The text engine to look up or load the glyph.
 * @param codepoint		The codepoint of the glyph to look up or load.
 * @param pixel_buffer	The handle to receive the pixel buffer of the found or loaded glyph of given codepoint.
 * @param glyph_size_x	The handle to receive the size of the X axis in pixels of the found or loaded glyph of given
 *						codepoint.
 * @return				The RGB565 MSB first bitmask pixel buffer of the glyph of the given codepoint.
 */
void private_lookup_or_load_glyph(
	const	esp_fast_text_engine_instance_t*	context,
			uint16_t							codepoint,
			uint16_t**							pixel_buffer,
			uint32_t*							glyph_size_x
);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ESP_FAST_TEXT_ENGINE_COMMON_H
