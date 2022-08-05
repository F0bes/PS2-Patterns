typedef struct {
	/** "FONTX2" id Identifier */
	char id[7];
	/** Name of the font */
	char name[9];
	/** Font Width XSize */
	unsigned char width;
	/** Font Height YSize */
	unsigned char height;
	/** Type of Font */
	unsigned char type;
	// Single-byte font headers end here
	/** Number of tables */
	unsigned char table_num;
	struct {
		/** Table[n] starting position */
		unsigned short start;
		/** Table[n] ending position */
		unsigned short end;
	} block[];
} fontx_hdr;