#include "string.h"
#include "esp_fast_text_engine.h"
#include "esp_fast_text_engine_common.h"

void private_lookup_or_load_glyph(
	const	esp_fast_text_engine_instance_t*	context,
	const	uint16_t							codepoint,
			uint16_t**							pixel_buffer,
			uint32_t*							glyph_size_x
) {
	// Extract necessary properties for glyph lookup.
	const uint32_t atlas_glyph_size = context->properties->atlas_glyph_size;

	// ASCII characters are resident in memory.
	if (codepoint < 128U) {
		// Return the pixel buffer and the size x of the glyph from corresponding ASCII resident atlas.
		*pixel_buffer = &	context->ascii_atlas[codepoint * atlas_glyph_size];
		*glyph_size_x =		context->ascii_atlas[codepoint];

		return;
	}

	// Lookup the glyph in the L1 IRAM LRU atlas.
	esp_fast_text_engine_lru_atlas_t* l1_lru_atlas = context->iram_lru_atlas;

	for (uint32_t slot = 0U; slot < l1_lru_atlas->slot_count; slot ++) {
		// Get the node of the corresponding
		esp_fast_text_engine_lru_node_t* l1_node = &l1_lru_atlas->nodes[slot];

		if (l1_node->codepoint == codepoint) {
			// Move the node to the head to reset its used state to recently used.
			private_move_lru_node_to_head(l1_lru_atlas, l1_node);

			// Return the pixel buffer and the size x of the glyph from L1 IRAM LRU atlas.
			*pixel_buffer = l1_node->pixel_buffer;
			*glyph_size_x = l1_node->size_x;

			return;
		}
	}

	// Lookup the glyph in the L2 PSRAM LRU atlas.
	esp_fast_text_engine_lru_atlas_t* l2_lru_atlas = context->psram_lru_atlas;

	for (uint32_t l2_slot = 0U; l2_slot < l2_lru_atlas->slot_count; l2_slot ++) {
		esp_fast_text_engine_lru_node_t* l2_node = &l2_lru_atlas->nodes[l2_slot];

		if (l2_node->codepoint == codepoint) {
			// Find the empty or least used L1 node to replace, it should always be the tail node.
			esp_fast_text_engine_lru_node_t* l1_node = l1_lru_atlas->tail;

			// Get the offset of the baked bitmask in pixel buffer for both L1 and L2 nodes, get the cached size x in L2 node.
					uint16_t*	dst_buffer = l1_node->pixel_buffer;
					uint16_t*	src_buffer = l2_node->pixel_buffer;
			const	uint32_t	src_size_x = l2_node->size_x;

			// Swap the content if the selected L1 node is not empty.
			if (l1_node->codepoint != EMPTY_CODEPOINT_SENTINEL) {
				uint16_t* swp_buffer = context->swap_buffer;

				// Swap the data of the two nodes. (L1 <=> L2)
				memcpy(swp_buffer, dst_buffer, atlas_glyph_size * 2U); // S = L1;
				memcpy(dst_buffer, src_buffer, atlas_glyph_size * 2U); // L1 = L2;
				memcpy(src_buffer, swp_buffer, atlas_glyph_size * 2U); // L2 = S;

				// Swap the codepoints.
				const uint32_t l1_codepoint	= l1_node->codepoint;
				l1_node->codepoint			= l2_node->codepoint;
				l2_node->codepoint			= l1_codepoint;

				// Swap the size x.
				const uint32_t l1_size_x	= l1_node->size_x;
				l1_node->size_x				= l2_node->size_x;
				l2_node->size_x				= l1_size_x;

				// Both nodes are not empty (not sentinel value), no need to set empty bitsets.
				// Move both nodes to the head of their linked list.
				private_move_lru_node_to_head(l1_lru_atlas, l1_node);
				private_move_lru_node_to_head(l2_lru_atlas, l2_node);
			} else {
				// Move the baked bitmask data to the L1 node.
				memcpy(
					/* dst_buffer	= */ dst_buffer,
					/* src_buffer	= */ src_buffer,
					/* length		= */ atlas_glyph_size * 2U
				);

				// Update the metadata of L1 node.
				l1_node->codepoint	= l2_node->codepoint;
				l1_node->size_x		= l2_node->size_x;

				// L2 node is empty now, set to sentinel value.
				l2_node->codepoint	= EMPTY_CODEPOINT_SENTINEL;
				l2_node->size_x		= 0U;

				// Move the used L1 node to the head of its linked list, move the empty L2 node to the tail of its linled list..
				private_move_lru_node_to_head(l1_lru_atlas, l1_node);
				private_move_lru_node_to_tail(l2_lru_atlas, l2_node);
			}

			// Return the pixel buffer and the size x of the glyph from the hoisted node in the L1 IRAM LRU atlas.
			*pixel_buffer = dst_buffer;
			*glyph_size_x = src_size_x;

			return;
		}
	}

	// Bake from tightly packed 1bpp LSBit first font glyph data.
	esp_fast_text_engine_lru_node_t* l1_lru_node = l1_lru_atlas->tail;

	// Move the original bitmask data of the L1 node to an empty or least used L2 node.
	if (l1_lru_node->codepoint != EMPTY_CODEPOINT_SENTINEL) {
		// Find the empty or least used L2 node to replace, it should always be the tail node.
		esp_fast_text_engine_lru_node_t* l2_lru_node = l2_lru_atlas->tail;

		// Move the baked bitmask data to the L2 node.
		memcpy(
			/* dst_buffer	= */ l2_lru_node->pixel_buffer,
			/* src_buffer	= */ l1_lru_node->pixel_buffer,
			/* length		= */ atlas_glyph_size * 2U
		);

		// Update the metadata of the L2 node.
		l2_lru_node->codepoint	= l1_lru_node->codepoint;
		l2_lru_node->size_x		= l1_lru_node->size_x;

		// Move the L2 node to the head of its linked list.
		private_move_lru_node_to_head(l2_lru_atlas, l2_lru_node);
	}

	// Update the codepoint of the L1 node.
	l1_lru_node->codepoint = codepoint;

	// Bake bitmask for the glyph of given codepoint.
	private_bake_atlas_from_1bpp(
		/* context		= */	context,
		/* codepoint	= */	codepoint,
		/* pixel_buffer	= */	l1_lru_node->pixel_buffer,
		/* glyph_size_x	= */ &	l1_lru_node->size_x
	);

	// Move the L1 node to the head of its linked list.
	private_move_lru_node_to_head(l1_lru_atlas, l1_lru_node);

	// Return the pixel buffer and the size x of the glyph from the baked L1 node.
	*pixel_buffer = l1_lru_node->pixel_buffer;
	*glyph_size_x = l1_lru_node->size_x;
}