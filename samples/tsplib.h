#define TL_MAGIC 0xA6DB9547UL

#define TL_STRLEN 1024

#define TL_TYPE_NONE             0	// Not specified
#define TL_TYPE_TSP              1	// Data for a symmetric traveling salesman problem
#define TL_TYPE_ATSP             2	// Data for an asymmetric traveling salesman problem
#define TL_TYPE_SOP              3	// Data for a sequential ordering problem
#define TL_TYPE_HCP              4	// Hamiltonian cycle problem data
#define TL_TYPE_CVRP             5	// Capacitated vehcle routing problem data
#define TL_TYPE_TOUR             6	// A collection of tours

#define TL_EWT_NONE              0
#define TL_EWT_EXPLICIT          1
#define TL_EWT_EUC_2D            2
#define TL_EWT_EUC_3D            3
#define TL_EWT_MAX_2D            4
#define TL_EWT_MAX_3D            5
#define TL_EWT_MAN_2D            6
#define TL_EWT_MAN_3D            7
#define TL_EWT_CEIL_2D           8
#define TL_EWT_GEO               9
#define TL_EWT_ATT              10
#define TL_EWT_XRAY1            11
#define TL_EWT_XRAY2            12
#define TL_EWT_SPECIAL          13

#define TL_EWF_NONE              0
#define TL_EWF_FUNCTION          1
#define TL_EWF_FULL_MATRIX       2
#define TL_EWF_UPPER_ROW         3
#define TL_EWF_LOWER_ROW         4
#define TL_EWF_UPPER_DIAG_ROW    5
#define TL_EWF_LOWER_DIAG_ROW    6
#define TL_EWF_UPPER_COL         7
#define TL_EWF_LOWER_COL         8
#define TL_EWF_UPPER_DIAG_COL    9
#define TL_EWF_LOWER_DIAG_COL   10

#define TL_EDF_NONE              0
#define TL_EDF_EDGE_LIST         1
#define TL_EDF_ADJ_LIST          2

#define TL_NCT_NONE              0
#define TL_NCT_TWOD_COORDS       1
#define TL_NCT_THREED_COORDS     2
#define TL_NCT_NO_COORDS         3

#define TL_DDT_NONE              0
#define TL_DDT_COORD_DISPLAY     1
#define TL_DDT_TWOD_DISPLAY      2
#define TL_DDT_NO_DISPLAY        3

typedef struct {
  int node_num;
  double r[3];
} tsplib_coord_t;

typedef struct {
  int node_num;
  int demand;
} tsplib_demand_t;

typedef struct {
  int node1, node2;
} tsplib_edgelist_t;

typedef struct {
  int node_num;
  int *adj;
  int nadj;
} tsplib_adj_t;

typedef struct {
  unsigned int magic;

  char name[TL_STRLEN];	// Identifies the data file
  int type;			// Specifies the type of data
  char comment[TL_STRLEN];	// Additional comments
  int dimension;		// Number of nodes (and depots)
  int capacity;		// truck capacity in a CVRP
  int edge_weight_type;
  int edge_weight_format;
  int edge_data_format;
  int node_coord_type;
  int display_data_type;
  tsplib_coord_t *coord;
  int *depot;
  int ndepot;
  tsplib_demand_t *demand;
  int ndemand;

  // to be written

} tsplib_t;

extern tsplib_t *tsplib_alloc(void);
extern void tsplib_free(tsplib_t * t);
extern int tsplib_loadfrom(tsplib_t * t, char *fn);
