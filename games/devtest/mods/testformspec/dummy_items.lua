-- This code adds dummy items that are supposed to be used in formspecs
-- for testing item_image formspec elements.

aperosengine.register_node("testformspec:node", {
	description = "Formspec Test Node",
	tiles = { "testformspec_node.png" },
	groups = { dig_immediate = 3, dummy = 1 },
})

aperosengine.register_craftitem("testformspec:item", {
	description = "Formspec Test Item",
	inventory_image = "testformspec_item.png",
	groups = { dummy = 1 },
})
