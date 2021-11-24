{
	"id": "NeShader",
	"version": 1,
	"name": "DefaultPBR_MR",
	"type": "graphics",
	"opaqueModules": [
		{ "stage": "vertex", "name": "DefaultPBR_O_VS" },
		{ "stage": "fragment", "name": "DefaultPBR_MR_O_FS" },
	],
	"transparentModules": [
		{ "stage": "vertex", "name": "DefaultPBR_T_VS" },
		{ "stage": "fragment", "name": "DefaultPBR_MR_T_FS" },
	]
}
