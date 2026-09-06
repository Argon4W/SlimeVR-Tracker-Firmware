#include "string.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_fast_text_engine.h"
#include "esp_fast_text_engine_common.h"

esp_err_t esp_fast_text_engine_new_text_engine_instance(
	esp_fast_text_engine_instance_t**				engine_instance_ret,
	esp_fast_text_engine_instance_configuration_t	engine_instance_configuration,
	esp_fast_text_engine_font_t						engine_instance_font
) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without a handle that receives the created text engine.
	ESP_RETURN_ON_FALSE(engine_instance_ret != NULL, ESP_ERR_INVALID_ARG, ESP_FAST_TEXT_ENGINE_TAG, "No esp_fast_text_engine_instance_t handle provided to receive the result when creating text engine instance.");

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_ESP_FAST_LCD_DEBUG_LOGGING
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "Reserving handles for text engine instance \"%s\".", engine_instance_configuration.name);
	#endif // CONFIG_ESP_FAST_LCD_DEBUG_LOGGING

	// Reserve handles for text engine instance.
	uint16_t*									engine_swap_buffer	= NULL;
	uint16_t*									engine_ascii_atlas	= NULL;
	uint32_t*									engine_ascii_size_x	= NULL;
	esp_fast_text_engine_instance_properties_t*	engine_properties	= NULL;
	esp_fast_text_engine_lru_atlas_t*			engine_l1_atlas		= NULL;
	esp_fast_text_engine_lru_atlas_t*			engine_l2_atlas		= NULL;
	esp_fast_text_engine_instance_t*			engine_instance		= NULL;

	// Get the properties from the font and configuration of the text engine instance.
	const uint32_t	font_max_size_x			= engine_instance_font			.size_x_max;
	const uint32_t	font_size_y				= engine_instance_font			.size_y;
	const uint32_t	font_size_multiplier	= engine_instance_configuration	.font_size_multiplier;
	const uint32_t	iram_atlas_slot_count	= engine_instance_configuration	.iram_atlas_slot_count;
	const uint32_t	psram_atlas_slot_count	= engine_instance_configuration	.psram_atlas_slot_count;
	const uint32_t	atlas_flags				= engine_instance_configuration	.atlas_flags;
	const char*		name					= engine_instance_configuration	.name;

	// Evaluate the font and configuration of the text engine instance.
	ESP_GOTO_ON_FALSE(font_max_size_x			> 0, ESP_ERR_INVALID_ARG, error, ESP_FAST_TEXT_ENGINE_TAG, "The size_x_max of the font must be greater than 0.");
	ESP_GOTO_ON_FALSE(font_size_y				> 0, ESP_ERR_INVALID_ARG, error, ESP_FAST_TEXT_ENGINE_TAG, "The size_y of the font must be greater than 0.");
	ESP_GOTO_ON_FALSE(font_size_multiplier		> 0, ESP_ERR_INVALID_ARG, error, ESP_FAST_TEXT_ENGINE_TAG, "The font_size_multiplier of the configuration must be greater than 0.");
	ESP_GOTO_ON_FALSE(iram_atlas_slot_count		> 0, ESP_ERR_INVALID_ARG, error, ESP_FAST_TEXT_ENGINE_TAG, "The iram_atlas_slot_count of the configuration must be greater than 0.");
	ESP_GOTO_ON_FALSE(psram_atlas_slot_count	> 0, ESP_ERR_INVALID_ARG, error, ESP_FAST_TEXT_ENGINE_TAG, "The psram_atlas_slot_count of the configuration must be greater than 0.");

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_ESP_FAST_LCD_DEBUG_LOGGING
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "Calculating properties for text engine instance \"%s\".", name);
	#endif // CONFIG_ESP_FAST_LCD_DEBUG_LOGGING

	// Calculate the properties of the atlases.
	const uint32_t atlas_glyph_size_x	= font_size_multiplier	* font_max_size_x;
	const uint32_t atlas_glyph_size_y	= font_size_multiplier	* font_size_y;
	const uint32_t atlas_glyph_size		= atlas_glyph_size_x	* atlas_glyph_size_y;

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_ESP_FAST_LCD_DEBUG_LOGGING
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "Allocating handles for text engine instance \"%s\".", name);
	#endif // CONFIG_ESP_FAST_LCD_DEBUG_LOGGING

	// Allocate properties and atlases of the engine instance.
	engine_instance		= calloc			(1U,						sizeof(esp_fast_text_engine_instance_t));
	engine_properties	= calloc			(1U,						sizeof(esp_fast_text_engine_instance_properties_t));
	engine_ascii_size_x	= calloc			(128U,						sizeof(uint32_t));
	engine_ascii_atlas	= heap_caps_calloc	(128U *	atlas_glyph_size,	sizeof(uint16_t), L2_ATLAS_CAPS);
	engine_swap_buffer	= heap_caps_calloc	(		atlas_glyph_size,	sizeof(uint16_t), L2_ATLAS_CAPS);

	// Check the allocations.
	ESP_GOTO_ON_FALSE(engine_instance		!= NULL, ESP_ERR_NO_MEM, error, ESP_FAST_TEXT_ENGINE_TAG, "Failed to create instance struct for text engine instance \"%s\".",				name);
	ESP_GOTO_ON_FALSE(engine_properties		!= NULL, ESP_ERR_NO_MEM, error, ESP_FAST_TEXT_ENGINE_TAG, "Failed to create properties for text engine instance \"%s\".",					name);
	ESP_GOTO_ON_FALSE(engine_ascii_atlas	!= NULL, ESP_ERR_NO_MEM, error, ESP_FAST_TEXT_ENGINE_TAG, "Failed to create resident ASCII atlas for text engine instance \"%s\".",			name);
	ESP_GOTO_ON_FALSE(engine_ascii_size_x	!= NULL, ESP_ERR_NO_MEM, error, ESP_FAST_TEXT_ENGINE_TAG, "Failed to create resident ASCII width cache for text engine instance \"%s\".",	name);
	ESP_GOTO_ON_FALSE(engine_swap_buffer	!= NULL, ESP_ERR_NO_MEM, error, ESP_FAST_TEXT_ENGINE_TAG, "Failed to create swap pixel buffer for text engine instance \"%s\".",			name);

	// Allocate and initialize the LRU atlases.
	ESP_GOTO_ON_ERROR(private_new_lru_atlas(&engine_l1_atlas, "IRAM LRU",	iram_atlas_slot_count,	atlas_glyph_size, L1_LRU_CAPS, L1_ATLAS_CAPS | atlas_flags),	error, ESP_FAST_TEXT_ENGINE_TAG, "Failed to initialize IRAM LRU atlas for text engine instance \"%s\".",	name);
	ESP_GOTO_ON_ERROR(private_new_lru_atlas(&engine_l2_atlas, "PSRAM LRU",	psram_atlas_slot_count,	atlas_glyph_size, L2_LRU_CAPS, L2_ATLAS_CAPS | atlas_flags),	error, ESP_FAST_TEXT_ENGINE_TAG, "Failed to initialize PSRAM LRU atlas for text engine instance \"%s\".",	name);

	#ifdef CONFIG_ESP_FAST_LCD_DEBUG_LOGGING
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "Filling properties for text engine instance \"%s\".", name);
	#endif // CONFIG_ESP_FAST_LCD_DEBUG_LOGGING

	// Fill the properties
	engine_properties->font						= engine_instance_font;
	engine_properties->font_size_multiplier		= engine_instance_configuration.font_size_multiplier;
	engine_properties->atlas_glyph_size_x		= atlas_glyph_size_x;
	engine_properties->atlas_glyph_size_y		= atlas_glyph_size_y;
	engine_properties->atlas_glyph_size			= atlas_glyph_size;
	engine_properties->name						= name;

	#ifdef CONFIG_ESP_FAST_LCD_DEBUG_LOGGING
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "Finalizing text engine instance \"%s\".", name);
	#endif // CONFIG_ESP_FAST_LCD_DEBUG_LOGGING

	// Fill the instance struct.
	engine_instance->properties			= engine_properties;
	engine_instance->iram_lru_atlas		= engine_l1_atlas;
	engine_instance->psram_lru_atlas	= engine_l2_atlas;
	engine_instance->ascii_atlas		= engine_ascii_atlas;
	engine_instance->ascii_size_x		= engine_ascii_size_x;
	engine_instance->swap_buffer		= engine_swap_buffer;

	// Bake ASCII characters into the resident ASCII
	#ifdef CONFIG_ESP_FAST_LCD_DEBUG_LOGGING
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "Baking resident ASCII characters atlas for text engine instance \"%s\".", name);
	#endif // CONFIG_ESP_FAST_LCD_DEBUG_LOGGING

	// Bake all ASCII characters.
	for (uint16_t codepoint = 0U; codepoint < 128U; codepoint ++) {
		// Bake the glyph into the resident ASCII atlas.
		private_bake_atlas_from_1bpp(
			/* context		= */ engine_instance,
			/* codepoint	= */ codepoint,
			/* pixel_buffer	= */ &engine_ascii_atlas	[codepoint * atlas_glyph_size],
			/* glyph_size_x	= */ &engine_ascii_size_x	[codepoint]
		);
	}

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_ESP_FAST_LCD_DEBUG_LOGGING
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "Text engine instance \"%s\" has been created.", name);
	#endif // CONFIG_ESP_FAST_LCD_DEBUG_LOGGING

	// Return the created text engine instance.
	*engine_instance_ret = engine_instance;

	return ret;

	// Resource cleanup when error occurred.
	error:

	// Log the error if LCD panel debug logging is enabled.
	#ifdef CONFIG_ESP_FAST_LCD_DEBUG_LOGGING
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "Error occurred while creating text engine instance \"%s\": %s", name, esp_err_to_name(ret));
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "Cleaning up resources.");
	#endif // CONFIG_ESP_FAST_LCD_DEBUG_LOGGING

	if (engine_l2_atlas)		private_del_lru_atlas	(engine_l2_atlas);		// Cleanup L2 PSRAM LRU atlas.
	if (engine_l1_atlas)		private_del_lru_atlas	(engine_l1_atlas);		// Cleanup L1 IRAM LRU atlas.
	if (engine_swap_buffer)		free					(engine_swap_buffer);	// Free the swap pixel buffer.
	if (engine_ascii_atlas)		free					(engine_ascii_atlas);	// Cleanup resident ASCII atlas.
	if (engine_properties)		free					(engine_properties);	// Cleanup the properties.
	if (engine_instance)		free					(engine_instance);		// Cleanup the instance struct.

	return ret;
}

esp_err_t esp_fast_text_engine_del_text_engine_instance(esp_fast_text_engine_instance_t* engine_instance_in) {
	// We cannot proceed without an allocated text engine.
	ESP_RETURN_ON_FALSE(engine_instance_in != NULL, ESP_ERR_INVALID_ARG, ESP_FAST_TEXT_ENGINE_TAG, "No esp_fast_text_engine_instance_t handle provided when releasing text engine instance.");

	// Get the name of the LCD panel device to release.
	const char* name = engine_instance_in->properties->name;

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_ESP_FAST_LCD_DEBUG_LOGGING
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "Releasing text engine instance \"%s\".", name);
	#endif // CONFIG_ESP_FAST_LCD_DEBUG_LOGGING

	// Releasing the direct allocations of the handles in the text engine instance.
	free(engine_instance_in->properties);
	free(engine_instance_in->ascii_atlas);
	free(engine_instance_in->ascii_size_x);
	free(engine_instance_in->swap_buffer);

	// Release the LRU atlas allocations.
	private_del_lru_atlas(engine_instance_in->iram_lru_atlas);
	private_del_lru_atlas(engine_instance_in->psram_lru_atlas);

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_ESP_FAST_LCD_DEBUG_LOGGING
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "Detaching all handles of text engine instance \"%s\"".", name);
	#endif // CONFIG_ESP_FAST_LCD_DEBUG_LOGGING

	// Detach all fields in the text engine.
	engine_instance_in->properties		= NULL;
	engine_instance_in->iram_lru_atlas	= NULL;
	engine_instance_in->psram_lru_atlas	= NULL;
	engine_instance_in->ascii_atlas		= NULL;
	engine_instance_in->ascii_size_x	= NULL;
	engine_instance_in->swap_buffer		= NULL;

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_ESP_FAST_LCD_DEBUG_LOGGING
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "Releasing the engine instance struct of text engine instance \"%s\".", name);
	#endif // CONFIG_ESP_FAST_LCD_DEBUG_LOGGING

	// Release the instance struct.
	free(engine_instance_in);

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_ESP_FAST_LCD_DEBUG_LOGGING
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "Text engine instance \"%s\" has been released.", name);
	#endif // CONFIG_ESP_FAST_LCD_DEBUG_LOGGING

	return ESP_OK;
}

esp_err_t private_new_lru_atlas(
			esp_fast_text_engine_lru_atlas_t**	atlas_ret,
	const	char*								atlas_name,
	const	uint32_t							atlas_slot_count,
	const	uint32_t							atlas_glyph_size,
	const	uint32_t							atlas_lru_flags,
	const	uint32_t							atlas_flags
) {
	// No atlas_ret check here, every call to private_new_lru_atlas is controlled by us.
	esp_err_t ret = ESP_OK;

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_ESP_FAST_LCD_DEBUG_LOGGING
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "Reserving handles for the %s atlas.", atlas_name);
	#endif // CONFIG_ESP_FAST_LCD_DEBUG_LOGGING

	// Reserve the handles for the atlas.
	esp_fast_text_engine_lru_atlas_t*	atlas			= NULL;
	esp_fast_text_engine_lru_node_t*	nodes			= NULL;
	uint16_t*							pixel_buffer	= NULL;

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_ESP_FAST_LCD_DEBUG_LOGGING
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "Allocating handles for the %s atlas.", atlas_name);
	#endif // CONFIG_ESP_FAST_LCD_DEBUG_LOGGING

	atlas			= calloc			(1,										sizeof(esp_fast_text_engine_lru_atlas_t));
	nodes			= heap_caps_calloc	(atlas_slot_count,						sizeof(esp_fast_text_engine_lru_node_t),	atlas_lru_flags);
	pixel_buffer	= heap_caps_calloc	(atlas_slot_count * atlas_glyph_size,	sizeof(uint16_t),							atlas_flags);

	// Check the allocations.
	ESP_GOTO_ON_FALSE(atlas			!= NULL, ESP_ERR_NO_MEM, error, ESP_FAST_TEXT_ENGINE_TAG, "Failed to create atlas struct for the %s atlas.",	atlas_name);
	ESP_GOTO_ON_FALSE(nodes			!= NULL, ESP_ERR_NO_MEM, error, ESP_FAST_TEXT_ENGINE_TAG, "Failed to create LRU linked list for the %s atlas.",	atlas_name);
	ESP_GOTO_ON_FALSE(pixel_buffer	!= NULL, ESP_ERR_NO_MEM, error, ESP_FAST_TEXT_ENGINE_TAG, "Failed to create pixel buffer for the %s atlas.",	atlas_name);

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_ESP_FAST_LCD_DEBUG_LOGGING
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "Initilaizing all nodes in the LRU linked list of the %s atlas.", atlas_name);
	#endif // CONFIG_ESP_FAST_LCD_DEBUG_LOGGING

	// Initialize all nodes to empty state and link all nodes together.
	for (uint32_t slot = 0; slot < atlas_slot_count; slot ++) {
		// Get the node of the slot.
		esp_fast_text_engine_lru_node_t* node = &nodes[slot];

		// Initialize the node to empty state then link the node to the linked list.
		node->prev			= (slot != 0)						? &nodes[slot - 1U] : NULL;
		node->next			= (slot != atlas_slot_count - 1)	? &nodes[slot + 1U] : NULL;
		node->pixel_buffer	= &pixel_buffer[slot * atlas_glyph_size];
		node->codepoint		= EMPTY_CODEPOINT_SENTINEL;
	}

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_ESP_FAST_LCD_DEBUG_LOGGING
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "Finalizing the %s atlas.", atlas_name);
	#endif // CONFIG_ESP_FAST_LCD_DEBUG_LOGGING

	// Fill the atlas struct.
	atlas->name			= atlas_name;
	atlas->head			= &nodes[0];
	atlas->tail			= &nodes[atlas_slot_count - 1];
	atlas->nodes		= nodes;
	atlas->pixel_buffer	= pixel_buffer;
	atlas->slot_count	= atlas_slot_count;

	// Return the allocated and initialized LRU atlas.
	*atlas_ret = atlas;

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_ESP_FAST_LCD_DEBUG_LOGGING
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "%s atlas has been created.", atlas_name);
	#endif // CONFIG_ESP_FAST_LCD_DEBUG_LOGGING

	return ret;

	// Resource cleanup when error occurred.
	error:

	// Log the error if LCD panel debug logging is enabled.
	#ifdef CONFIG_ESP_FAST_LCD_DEBUG_LOGGING
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "Error occurred while creating %s atlas: %s", atlas_name, esp_err_to_name(ret));
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "Cleaning up resources.");
	#endif // CONFIG_ESP_FAST_LCD_DEBUG_LOGGING

	if (pixel_buffer)	free(pixel_buffer);	// Cleanup the pixel buffer of the atlas.
	if (nodes)			free(nodes);		// Cleanup the LRU linked list of the atlas.
	if (atlas)			free(atlas);	// Cleanup the atlas struct of the atlas.

	return ret;
}

esp_err_t private_del_lru_atlas(esp_fast_text_engine_lru_atlas_t* atlas_in) {
	// We cannot proceed without an allocated atlas.
	ESP_RETURN_ON_FALSE(atlas_in != NULL, ESP_ERR_INVALID_ARG, ESP_FAST_TEXT_ENGINE_TAG, "No esp_fast_text_engine_lru_atlas_t handle provided when releasing LRU atlas.");

	// Get the name of the LCD panel device to release.
	const char* name = atlas_in->name;

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_ESP_FAST_LCD_DEBUG_LOGGING
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "Releasing the %s atlas.", name);
	#endif // CONFIG_ESP_FAST_LCD_DEBUG_LOGGING

	// Free the handles in the atlas.
	free(atlas_in->pixel_buffer);
	free(atlas_in->nodes);

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_ESP_FAST_LCD_DEBUG_LOGGING
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "Detaching all handles of %s atlas.", name);
	#endif // CONFIG_ESP_FAST_LCD_DEBUG_LOGGING

	// Detaching all fields in atlas.
	atlas_in->name			= NULL;
	atlas_in->head			= NULL;
	atlas_in->tail			= NULL;
	atlas_in->nodes			= NULL;
	atlas_in->pixel_buffer	= NULL;

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_ESP_FAST_LCD_DEBUG_LOGGING
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "Releasing the atlas struct of %s atlas.", name);
	#endif // CONFIG_ESP_FAST_LCD_DEBUG_LOGGING

	// Release the atlas struct.
	free(atlas_in);

	// Log the progress if LCD panel debug logging is enabled.
	#ifdef CONFIG_ESP_FAST_LCD_DEBUG_LOGGING
		ESP_LOGD(ESP_FAST_TEXT_ENGINE_TAG, "%s atlas has been released.", name);
	#endif // CONFIG_ESP_FAST_LCD_DEBUG_LOGGING

	return ESP_OK;
}