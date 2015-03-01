
#include "DXUT.h"
#include "d3dx11effect.h"
#include "DXUTcamera.h"
#include <AntTweakBar.h>
#include "TransferFunctionEditor.h"
#include <string.h>
//#include <vld.h>
//SAFE_GET_* macros
#define SAFE_GET_PASS(Technique, name, var)   {assert(Technique!=NULL); var = Technique->GetPassByName( name );	  assert(var->IsValid());}
#define SAFE_GET_TECHNIQUE(effect, name, var) {assert(effect   !=NULL); var = effect->GetTechniqueByName( name ); assert(var->IsValid());}
#define DEG2RAD( a ) ( a * D3DX_PI / 180.f )
#define MAXPROBES 5
#define NUMOFPARTICLES 100000
#define COLOR1 D3DXVECTOR4(0,1,0,1)
#define COLOR2 D3DXVECTOR4(0,0,1,1)
#define COLOR3 D3DXVECTOR4(0,1,1,1)
#define COLOR4 D3DXVECTOR4(1,0,0,1)
#define COLOR5 D3DXVECTOR4(1,1,0,1)

//Global variables for the effect
ID3DX11Effect*          g_Effect             = nullptr;
ID3DX11EffectTechnique* g_RenderTech         = nullptr;
ID3DX11EffectPass*      g_VolumePassISO = nullptr;
ID3DX11EffectPass*      g_VolumePassDVR = nullptr;
ID3DX11EffectPass*      g_ArrowsPass = nullptr;
ID3DX11EffectPass*      g_DomainPass = nullptr;
ID3DX11EffectPass*      g_ParticlesPass = nullptr;
ID3DX11EffectPass*      g_ProbesPass = nullptr;
ID3DX11EffectPass*      g_Velocity = nullptr;
ID3DX11EffectPass*      g_Vorticity = nullptr;
ID3DX11EffectPass*      g_Divergence = nullptr;
ID3DX11EffectPass*      g_Q = nullptr;
ID3DX11EffectPass*      g_StreamLines = nullptr;
ID3DX11EffectPass*      g_Integrate = nullptr;
ID3DX11EffectPass*      g_IntegrateRibbons = nullptr;
ID3DX11EffectPass*      g_Ribbons = nullptr;
ID3DX11EffectPass*      g_StreakLines = nullptr;
ID3DX11EffectPass*      g_IntegrateStreakLines = nullptr;
ID3DX11EffectPass*      g_Opacity = nullptr;
//Global variable for camera
CModelViewerCamera* g_Camera = new CModelViewerCamera();
//FPS camera 
CFirstPersonCamera* f_Camera= new CFirstPersonCamera();
bool fps=false;
float cameraRot=0.005f;
float cameraTrans=5.0f;
int fileIndexStep;
//array of arrays of bytes
char** scaledValues;
//flag if mesh loaded
bool meshLoaded;
//size of point in texture
int pointSize = 1;
//vector parameters
float sliceWidth,sliceHeight,sliceDepth,timeStep;
float resX,resY,resZ;
ID3DX11EffectScalarVariable	*gresX,*gresY,*gresZ;
//string mesh data file
int volumeVars[4];
//global variables for camera matrices
ID3DX11EffectMatrixVariable*         g_pWorldViewProj = nullptr;
ID3DX11EffectMatrixVariable*         g_pWorldViewProjInverse = nullptr;
ID3DX11EffectMatrixVariable*         g_pViewVariable = nullptr;
ID3DX11EffectMatrixVariable*         g_pProjectionVariable = nullptr;
ID3DX11EffectMatrixVariable*         g_scalingMatrix = nullptr;
D3DXMATRIX                          g_World; 
ID3DX11EffectMatrixVariable*         g_CamWorldMatrix = nullptr;
ID3DX11EffectScalarVariable*		shininess=nullptr;
ID3DX11EffectScalarVariable*		gdispValueMin=nullptr;
ID3DX11EffectScalarVariable*		gdispValueMax=nullptr;
ID3DX11EffectScalarVariable*		gstepSize=nullptr;
ID3DX11EffectScalarVariable*		gpickedZ=nullptr;
ID3DX11EffectScalarVariable*		gtimestep=nullptr;
ID3DX11EffectScalarVariable*		grenderstatic=nullptr;
ID3DX11EffectScalarVariable*		gsliceThicknessX=nullptr;
ID3DX11EffectScalarVariable*		gsliceThicknessY=nullptr;
ID3DX11EffectScalarVariable*		gsliceThicknessZ=nullptr;
ID3DX11EffectScalarVariable*		gmaxAge=nullptr;
ID3DX11EffectVectorVariable* color = nullptr;
ID3DX11EffectScalarVariable* gflipValue = nullptr;
float colorArray[3];
D3DXMATRIX* cameraMatrix=new D3DXMATRIX();
D3DXMATRIX* cameraMatrixInverse=new D3DXMATRIX();
D3DXMATRIX* scalingMatrix=new D3DXMATRIX();
ID3DX11EffectVectorVariable* LightPosition;
ID3DX11EffectVectorVariable* LightDirection;
ID3DX11EffectVectorVariable* CameraAt;
//texture
ID3D11Texture3D* Texture3D; //first texture
ID3D11ShaderResourceView* shaderResourceViewTexture; 
ID3DX11EffectShaderResourceVariable* gTexture3D= nullptr;

//texture weight
ID3DX11EffectScalarVariable* textureWeight, * textureWeightNext;

ID3D11Texture3D* Texture3DNext; //second texture
ID3D11ShaderResourceView* shaderResourceViewTextureNext; 
ID3DX11EffectShaderResourceVariable* gTexture3DNext= nullptr;
//ID3D11Texture1D* Texture1D;
ID3D11ShaderResourceView* shaderResourceViewTexture1D; 
ID3DX11EffectShaderResourceVariable* gTexture1D= nullptr;


//GUI
TwBar *g_Bar;
bool renderFlow=true;
bool renderMesh=true;
bool renderArrows=false;
bool renderParticles=false;
int shineFactor=5;
int shineDisplay=20;
float R=0.1f;float G=0.1f;float B=0.1f;
float dispValueMin=0.1f;
float dispValueMax=1.0f;
float stepSize=0.003f;
float currentStepSize = 0;
bool play=true;
int speed=5;
int currentTime = 2;
int pickedZ=-1;
TransferFunctionEditor* tfe=new TransferFunctionEditor(200,200);
bool renderStaticVolume;
//mesh loading
const char* name=nullptr;
long nvertices, ntriangles;
int* indices;int* domainIndices;
long counter=0,counter2=0;
ID3D11Buffer* g_VertexBuffer=nullptr;
ID3D11Buffer* g_IndexBuffer=nullptr;

ID3DX11EffectPass*		g_MeshPass			 = nullptr;
ID3D11InputLayout*		g_InputLayout		 = nullptr;
ID3D11InputLayout*		m_InputLayout		 =nullptr;


//------------------------------
// structures
//--------------------
struct Vertex { 
	D3DXVECTOR4 Pos; // Position 
	D3DXVECTOR3 Nor; // Normal 
};Vertex* vertices; 
Vertex* domainVertices;
ID3D11Buffer *m_vertexBuffer, *m_indexBuffer; //for domain box rendering
struct Particle{
	D3DXVECTOR4 pos ;
	D3DXVECTOR4 vec ;
	D3DXVECTOR4 worldPos ;
	float age;
}; 

//-----------------------------------------
//FUNCTIONS 
//-----------------------------------------
void InitGUI(ID3D11Device* pd3dDevice);
HRESULT CreateBuffers(ID3D11Device* pd3dDevice,HRESULT hr);
int ParseData();
HRESULT CreateInputLayout(ID3D11Device* pd3dDevice,HRESULT hr);
void initDomain(ID3D11Device* pd3dDevice);

void Spawn(Particle* p,D3DXVECTOR3 min,D3DXVECTOR3 max,float maxage,int count);
//enum
enum dataFormat
{
	BYTEFormat, UCHARFormat
};

ID3D11ShaderResourceView* nullView=nullptr;
ID3D11UnorderedAccessView *unorderedNullView=nullptr;

ID3DX11EffectUnorderedAccessViewVariable* gparticlesResource=nullptr;
ID3DX11EffectShaderResourceVariable* gparticlesBufferVariable,* grandomParticlesBufferVariable=nullptr;
float minAge=0;
float ageTrack=minAge;
int gridX,gridY,gridZ;
ID3DX11EffectScalarVariable* ggridX,*ggridY,*ggridZ=nullptr;
ID3DX11EffectVectorVariable* gMouseMin,*gMouseMax,*gProbeColor,*gParticlesColor;
D3DXVECTOR3 mousePointsMin;
D3DXVECTOR3 mousePointsMax;
int leftMouseButtonClicked = -1;


struct Probe{
	
	D3DXVECTOR3 minPoint,maxPoint,size;
	D3DXVECTOR4 color;
	int count;
	int* seperators;
	float maxAge,ageTrack;
	Particle* particles, *randomParticles,*streakLines;
	Vertex* lines,*ribbons,*spawn;
	ID3D11Buffer *buffer, *randomBuffer,*linesBuffer,*ribbonsBuffer,*streakBuffer,*spawnBuffer,*sepBuffer;
	ID3D11UnorderedAccessView* uav,*linesUAV,*ribbonsUAV,*streakUAV,*spawnUAV,*sepUAV;
	ID3D11ShaderResourceView* srv,*randomsrv,*linesSRV,*ribbonsSRV,*streakSRV,*spawnSRV,*sepSRV;

};
Probe* probes;
int numOfProbes=0;
int selectedIndex=-1;
D3DXVECTOR3 temp;
HRESULT CreateUnorderedBuffer(ID3D11Device* pd3dDevice,HRESULT hr,Probe* p);
bool moving=false;
bool scaling= false;
bool changeZ=false;
ID3D11Texture3D* RWTexture3D;
ID3D11Texture2D* DepthTexture;
ID3D11UnorderedAccessView* shaderResourceViewRWTexture; 
ID3DX11EffectUnorderedAccessViewVariable* gRWTexture3D= nullptr;
ID3D11ShaderResourceView* DepthSRV, *scalarTextureSVR=nullptr;
ID3DX11EffectShaderResourceVariable* gDepthTexture,*gscalarTexture;
ID3D11Resource* DepthResource;
bool renderVelocity=false;
bool renderVorticity=false;
bool renderDivergence=false;
bool renderQ=false;
bool VolumeRenderingISO=false;
bool VolumeRenderingDVR=false;
bool DVR=false;
int flipValue;
bool flip=false;
//line streams
int numberOfLines=1000;
int lengthOfLine=100;
int previousNumberOfLines=numberOfLines;
int previousLengthOfLine=lengthOfLine;
float integrationStepSize=0.01;

void createLinesBuffer(ID3D11Device* pd3dDevice,Probe* p,int n);
void SpawnLines(Probe* probe,int n);
void resizeLinesBuffer(ID3D11Device* pd3dDevice,int n);
void createIndexBuffer(ID3D11Device* pd3dDevice);

ID3DX11EffectUnorderedAccessViewVariable* linesUAVvariable;
ID3DX11EffectShaderResourceVariable* linesSRVvariable;
ID3DX11EffectUnorderedAccessViewVariable* ribbonsUAVvariable;
ID3DX11EffectShaderResourceVariable* ribbonsSRVvariable;
bool renderStreamLines=false;
bool renderStreamTubes=false;
bool renderRibbons=false;
ID3DX11EffectScalarVariable*		gintegrationStepSize=nullptr;
ID3DX11EffectScalarVariable*		glengthOfLine=nullptr;
ID3DX11EffectScalarVariable*		gnumberOfLines=nullptr;
ID3DX11EffectScalarVariable*		giteration=nullptr;
ID3DX11EffectScalarVariable*		gseperator=nullptr;
bool updateBuffer=false;
float ribbonsWidth=0.01;
float previousRibbonsWidth=ribbonsWidth;
ID3D11Buffer* indexBuffer=nullptr;
//streaklines
void SpawnStreakLines(Probe* probe);
void initAges(Probe* probe);
void initSeperators(Probe* probe);
void createStreakLinesBuffer(ID3D11Device* pd3dDevice,Probe* p);
void resizeStreakLinesBuffer(ID3D11Device* pd3dDevice);
ID3DX11EffectUnorderedAccessViewVariable* StreaklinesUAVvariable,*spawnUAVvariable,*sepUAVvariable;
ID3DX11EffectShaderResourceVariable* StreaklinesSRVvariable,*spawnSRVvariable,*sepSRVvariable;
bool renderStreakLines=false;
bool renderStreakTubes=false;
float seedingInterval=0.05;
int streakLinesLength=50;
float lifetime=seedingInterval*(streakLinesLength-1);
int previousstreakLinesLength=streakLinesLength;
ID3DX11EffectScalarVariable*		gseedingInterval=nullptr;
ID3DX11EffectScalarVariable*		gstreakLinesLength=nullptr;
ID3DX11EffectScalarVariable*		glifetime=nullptr;
ID3DX11EffectScalarVariable*		gtimeSinceLastFrame=nullptr;
//smoke surfaces variables
bool useFade=true,useDensity=true,useShape=true; bool verticalLine=true;
ID3DX11EffectScalarVariable* guseFade,*guseDensity,*guseShape;