//--------------------------------------------------------------------------------------
// File: main.cpp
//
// Simple starting point for our Direct3D 11 visualization tool.
// Based on EmptyProject11 from the DirectX Sample Browser.
//
//--------------------------------------------------------------------------------------

#include "src/tools.h"
#include <rply.h>
#include <cstdint>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include "main.h"
#include <fstream>
using namespace std;
#include <iostream>
#include <sstream>
#include <tchar.h>
#include <Windows.h>
#include <WindowsX.h>
#include <stdio.h>
#include <Mmsystem.h>
string meshFilePathName="";
string dataFormat="";


//------------------------
// call back functions for reading ply files 
//------------------------
static int vertex_cb(p_ply_argument argument) {
	long eol;
	ply_get_argument_user_data(argument, NULL, &eol);
	p_ply_property *propertyy=(p_ply_property*)calloc(1,sizeof(p_ply_property));
	long *length=NULL,*value_index=NULL;
	ply_get_argument_property(argument,propertyy,length,value_index);
	e_ply_type *type=(e_ply_type*)calloc(1,sizeof(e_ply_type)),*length_type=(e_ply_type*)calloc(1,sizeof(e_ply_type)), *value_type=(e_ply_type*)calloc(1,sizeof(e_ply_type));
	ply_get_property_info(*propertyy,&name,type,length_type,value_type);
	if(strcmp (name,"x")==0)
		vertices[counter].Pos.x=static_cast<float> (ply_get_argument_value(argument));
	if(strcmp (name,"y")==0)
		vertices[counter].Pos.y=static_cast<float> (ply_get_argument_value(argument));
	if(strcmp (name,"z")==0)
		vertices[counter].Pos.z=static_cast<float> (ply_get_argument_value(argument));
	if(strcmp (name,"nx")==0)
		vertices[counter].Nor.x=static_cast<float> (ply_get_argument_value(argument));
	if(strcmp (name,"ny")==0)
		vertices[counter].Nor.y=static_cast<float> (ply_get_argument_value(argument));
	if(strcmp (name,"nz")==0)
	{
		vertices[counter].Nor.z=static_cast<float> (ply_get_argument_value(argument));
		counter=counter+1;
	}
	if (eol) 
		printf("\n");
	else printf(" ");
	delete type;
	delete length_type;
	delete value_type;
	delete propertyy;

	return 1;
}


static int face_cb(p_ply_argument argument) {
	long length, value_index;
	ply_get_argument_property(argument, NULL, &length, &value_index);
	if(value_index==-1)
		return 1;
	else
	{
		indices[counter2]=static_cast<int> (ply_get_argument_value(argument));
		counter2++;
	}
	return 1;
}



void getVolumeData(OPENFILENAME ofn, char* scaledValuesT, int sizeOfBufferT, int pointSizeT, bool needPaddingT)
{
	char* dataBuffer;
	// We have an 8-bit RAW file
	ifstream rawFile;
	char s [200];
	for(int i=0;i<200;i++)
		s[i] = ofn.lpstrFile[i];
	string rawFileNameT = (string)s;
	rawFile.open(rawFileNameT,ifstream::binary);


	// Scale the values to a [0,1] range
	int index = 0;
	if(!needPaddingT)
	{
		rawFile.read(scaledValuesT,sizeOfBufferT);				
	}
	else
	{
		int sizeT = (sizeOfBufferT *3)/4;
		dataBuffer = (char*) malloc(sizeT);
		rawFile.read(dataBuffer,sizeT);
		for (int i = 0; i < (sizeOfBufferT / pointSizeT) / 4; ++i)
		{
			for(int j = 0;j< pointSizeT * 3;j++)
			{
				scaledValuesT[(i * pointSizeT * 4)+ j] = dataBuffer[(i * pointSizeT * 3)+ j];
			}
			for(int j = pointSizeT * 3;j< pointSizeT * 4;j++)
			{
				scaledValuesT[(i * pointSizeT * 4)+ j] = 0;
			}		
		}	
	}
	delete [] dataBuffer;
}
void buildTexture(ID3D11Device* pd3dDevice, OPENFILENAME ofn)        
{
	D3D11_TEXTURE3D_DESC* desc=new D3D11_TEXTURE3D_DESC(); 
	int sizeOfBuffer;

	desc->MipLevels          = 1; 
	desc->Usage              = D3D11_USAGE_DEFAULT ; 
	desc->BindFlags          = D3D11_BIND_SHADER_RESOURCE ;
	//desc->CPUAccessFlags     = D3D11_CPU_ACCESS_WRITE;  
	desc->MiscFlags          = 0;

	char line[256];

	ifstream datFile;
	char objectFileName[200];
	char meshFileName[200];
	char tempFileName [200];
	for(int i=0;i<200;i++)
		tempFileName[i] = ofn.lpstrFile[i];
	string datFileName = (string)tempFileName;
	datFile.open(datFileName,ifstream::in);
	volumeVars[1] = -1;
	meshLoaded = false;
	bool needPadding;
	if(datFile.is_open())
	{
		while(!datFile.eof())
		{
			datFile.getline(line,256);
			string s = line;
			s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
			istringstream iss(s);

			string firstSubstring;
			iss >> firstSubstring;
			if (firstSubstring=="ObjectFileName:")
			{
				iss >> objectFileName;
			}
			if (firstSubstring=="MeshFileName:")
			{
				iss >> meshFileName;
				meshLoaded = true;
			}
			else if (firstSubstring=="Resolution:")
			{
				iss >> desc->Width;
				iss >> desc->Height;
				iss >> desc->Depth;
				resX=desc->Width;
				resY=desc->Height;
				resZ=desc->Depth;
			}
			else if (firstSubstring=="SliceThickness:")
			{
				iss >> sliceWidth;
				iss >> sliceHeight;
				iss >> sliceDepth;
			}
			else if (firstSubstring=="Timestep:")
			{
				iss >> timeStep;				
			}
			else if (firstSubstring=="Format:")
			{
				iss >> dataFormat;
				if(dataFormat == "BYTE")
					desc->Format = DXGI_FORMAT_R8_UNORM;					
				else if (dataFormat == "UCHAR")
					desc->Format = DXGI_FORMAT_R8_UNORM;
				else if (dataFormat == "half3")
				{
					desc->Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
					pointSize = 2;
					needPadding = true;
				}
				else if (dataFormat == "half4")
				{
					desc->Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
					pointSize = 2;
					needPadding = false;
				}
				else if (dataFormat == "float3")
				{
					desc->Format = DXGI_FORMAT_R32G32B32A32_FLOAT;				
					pointSize = 4;
					needPadding = true;
				}
				else if (dataFormat == "float4")
				{
					desc->Format = DXGI_FORMAT_R32G32B32A32_FLOAT;				
					pointSize = 4;
					needPadding = false;
				}
			}
			else if (firstSubstring=="ObjectIndices:")
			{
				iss >> volumeVars[1];
				iss >> volumeVars[2];

				if(!iss.eof())
					iss >> volumeVars[3];
				else
					volumeVars[3] = 1;
			}

		}	
		//the two weights sum always is the file index step
		textureWeight->SetFloat(timeStep);
		textureWeightNext->SetFloat(0);
		if(volumeVars[1] < 0)
			renderStaticVolume = true;
		else
			renderStaticVolume = false;
		if(!renderStaticVolume)
			volumeVars[0] = 1 + (volumeVars[2] - volumeVars[1])/volumeVars[3];
		else
			volumeVars[0] = 1;
		sizeOfBuffer = desc->Width * desc->Height * desc->Depth;
		sizeOfBuffer = sizeOfBuffer * pointSize * 4;		
		scaledValues =(char**)malloc(volumeVars[0]*sizeof(char*));
		char fileName[200];
		wchar_t ff[200];
		for(int j=0;j<ofn.nFileOffset;j++)
			ff[j] = ofn.lpstrFile[j];
		for(int j=ofn.nFileOffset;j<200;j++)
			ff[j] = meshFileName[j - ofn.nFileOffset];
		for(int i=0;i<200;i++)
			meshFileName[i] = (char)ff[i];
		meshFilePathName = (string)meshFileName;
		for(int i=0;i<volumeVars[0];i++)
		{
			scaledValues[i] = (char*) malloc(sizeOfBuffer);
			sprintf_s(fileName,objectFileName,(i * volumeVars[3]) + volumeVars[1]);
			OPENFILENAME ofn2 = ofn;
			for(int j=ofn.nFileOffset;j<200;j++)
				ff[j] = fileName[j - ofn.nFileOffset];
			ofn2.lpstrFile = ff;			
			getVolumeData(ofn2,scaledValues[i],sizeOfBuffer, pointSize, needPadding);
		}
		//create texture 
		// Define initial data
		D3D11_SUBRESOURCE_DATA* TextureData= new D3D11_SUBRESOURCE_DATA();
		//texture 1
		if(!renderStaticVolume)
			TextureData->pSysMem=scaledValues[currentTime];
		else
		{
			TextureData->pSysMem=scaledValues[0];
			textureWeightNext->SetFloat(1.0);
			textureWeight->SetFloat(1.0);
		}
		TextureData->SysMemPitch=(desc->Width)*4*pointSize;
		TextureData->SysMemSlicePitch=(desc->Width*desc->Height)*4*pointSize;
		pd3dDevice->CreateTexture3D(desc,TextureData,&Texture3D); 
		D3D11_SHADER_RESOURCE_VIEW_DESC* shaderResourceViewDesc = new D3D11_SHADER_RESOURCE_VIEW_DESC();
		shaderResourceViewDesc->Format=desc->Format;
		shaderResourceViewDesc->ViewDimension=D3D11_SRV_DIMENSION_TEXTURE3D;
		shaderResourceViewDesc->Texture3D.MostDetailedMip=0;
		shaderResourceViewDesc->Texture3D.MipLevels=1;
		pd3dDevice->CreateShaderResourceView(Texture3D,shaderResourceViewDesc,&shaderResourceViewTexture); 
		//texture 2
		if(!renderStaticVolume)
			TextureData->pSysMem=scaledValues[(currentTime+1)%volumeVars[0]];
		else
			TextureData->pSysMem=scaledValues[0];
		pd3dDevice->CreateTexture3D(desc,TextureData,&Texture3DNext); 
		pd3dDevice->CreateShaderResourceView(Texture3DNext,shaderResourceViewDesc,&shaderResourceViewTextureNext);
		volumeVars[1] = desc->Width;
		volumeVars[2] = desc->Height;

		//create RWTexture3D<float>
		D3D11_TEXTURE3D_DESC* td = new D3D11_TEXTURE3D_DESC(); // init to binary zero = default values
		td->Usage = D3D11_USAGE_DEFAULT;
		td->Depth=desc->Depth;
		td->Height=desc->Height;
		td->Width=desc->Width;
		td->CPUAccessFlags=0;
		td->MiscFlags=0;
		td->BindFlags = D3D11_BIND_UNORDERED_ACCESS| D3D11_BIND_SHADER_RESOURCE;
		td->Format=DXGI_FORMAT_R32_FLOAT;
		td->MipLevels=1;
		float* tmp=(float*)calloc(desc->Depth*desc->Height*desc->Width,sizeof(float));
		for(int i=0;i<desc->Depth*desc->Height*desc->Width;i++)
		{
			tmp[i]=0.3;
		}
		TextureData->pSysMem=tmp;
		TextureData->SysMemPitch=td->Width*sizeof(float);
		TextureData->SysMemSlicePitch=(td->Width*td->Height)*sizeof(float);
		pd3dDevice->CreateTexture3D(td,TextureData,&RWTexture3D);
		//createUAV

		D3D11_UNORDERED_ACCESS_VIEW_DESC* vd=new D3D11_UNORDERED_ACCESS_VIEW_DESC();
		vd->Format=DXGI_FORMAT_R32_FLOAT;
		vd->Buffer.NumElements=desc->Height*desc->Depth*desc->Width;
		vd->ViewDimension=D3D11_UAV_DIMENSION_TEXTURE3D;
		vd->Texture3D.MipSlice=0;
		vd->Texture3D.FirstWSlice=0;
		vd->Texture3D.WSize=-1;
		pd3dDevice->CreateUnorderedAccessView(RWTexture3D,vd,&shaderResourceViewRWTexture);
		//create SRV
		D3D11_SHADER_RESOURCE_VIEW_DESC* sd=new D3D11_SHADER_RESOURCE_VIEW_DESC();
		sd->Format=DXGI_FORMAT_R32_FLOAT;
		sd->ViewDimension=D3D11_SRV_DIMENSION_TEXTURE3D;
		sd->Texture3D.MipLevels=1;
		sd->Texture3D.MostDetailedMip=0;
		pd3dDevice->CreateShaderResourceView(RWTexture3D,sd,&scalarTextureSVR);

		delete sd;
		delete vd;
		delete td;
		delete [] tmp;
		delete(TextureData);
		delete(shaderResourceViewDesc);
		delete desc;

	}
}






//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
	DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
	
	// any D3D11 device is OK
	return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
	pDeviceSettings->d3d11.AutoDepthStencilFormat=DXGI_FORMAT_D32_FLOAT;
	return true;
}

//used when reloading volumes
void Cleanup()
{

	SAFE_RELEASE(Texture3D);
	SAFE_RELEASE(Texture3DNext);
	SAFE_RELEASE(shaderResourceViewTexture);
	SAFE_RELEASE(shaderResourceViewTextureNext);
	for(int i=0;i<volumeVars[0];i++)
	{
		free (scaledValues[i]);
	}
	free(scaledValues);

}

//restarts particles
void TW_CALL restart(void* pd3dDevice)
{
	updateBuffer=true;
}

//switches streaklines/surfaces orientation
void TW_CALL switchVerticalHorizontal(void* pd3dDevice)
{
	verticalLine=!verticalLine;
	updateBuffer=true;
}

//removes the highlighted probe
void TW_CALL removeProbe(void* pd3dDevice)
{
	if(selectedIndex<0 || selectedIndex>=numOfProbes) return;
		SAFE_RELEASE(probes[selectedIndex].srv);
		SAFE_RELEASE(probes[selectedIndex].uav);
		SAFE_RELEASE(probes[selectedIndex].randomsrv);
		SAFE_RELEASE(probes[selectedIndex].buffer);
		SAFE_RELEASE(probes[selectedIndex].randomBuffer);
		SAFE_RELEASE(probes[selectedIndex].linesBuffer);
		SAFE_RELEASE(probes[selectedIndex].ribbonsBuffer);
		SAFE_RELEASE(probes[selectedIndex].linesSRV);
		SAFE_RELEASE(probes[selectedIndex].ribbonsSRV);
		SAFE_RELEASE(probes[selectedIndex].linesUAV);
		SAFE_RELEASE(probes[selectedIndex].ribbonsUAV);
		SAFE_RELEASE(probes[selectedIndex].streakUAV);
		SAFE_RELEASE(probes[selectedIndex].streakSRV);
		SAFE_RELEASE(probes[selectedIndex].streakBuffer);
		SAFE_RELEASE(probes[selectedIndex].spawnBuffer);
		SAFE_RELEASE(probes[selectedIndex].spawnUAV);
		SAFE_RELEASE(probes[selectedIndex].spawnSRV);
		SAFE_RELEASE(probes[selectedIndex].sepBuffer);
		SAFE_RELEASE(probes[selectedIndex].sepUAV);
		SAFE_RELEASE(probes[selectedIndex].sepSRV);
		delete [] probes[selectedIndex].particles;
		delete [] probes[selectedIndex].randomParticles;
		delete [] probes[selectedIndex].lines;
		delete [] probes[selectedIndex].ribbons;
		delete [] probes[selectedIndex].streakLines;
		delete [] probes[selectedIndex].spawn;
		delete [] probes[selectedIndex].seperators;
	//	delete &probes[selectedIndex];
	for(int i=selectedIndex;i<numOfProbes-1;i++)
	{
		probes[i]=probes[i+1];
	}
	numOfProbes--;
	selectedIndex--;
}

//adds a probe
void TW_CALL addProbe(void* pd3dDevice)
{	
	HRESULT hr=0;
	if(numOfProbes==5) return;
	
	if(numOfProbes==0)
		probes[numOfProbes].color=COLOR2;
	if(numOfProbes==1)
		probes[numOfProbes].color=COLOR1;
	if(numOfProbes==2)
		probes[numOfProbes].color=COLOR3;
	if(numOfProbes==3)
		probes[numOfProbes].color=COLOR4;
	if(numOfProbes==4)
		probes[numOfProbes].color=COLOR5;

	probes[numOfProbes].size=D3DXVECTOR3(resX/4,resY/2,resZ/4);
	probes[numOfProbes].count=100000;
	probes[numOfProbes].maxAge=3;
	probes[numOfProbes].minPoint=D3DXVECTOR3(resX/10,resY/4,resZ/4);
	probes[numOfProbes].maxPoint=D3DXVECTOR3((resX/4)+probes[numOfProbes].size.x,(resY/4)+probes[numOfProbes].size.y,(resZ/4)+probes[numOfProbes].size.z);
	probes[numOfProbes].particles=(Particle*)malloc(sizeof(Particle)*probes[numOfProbes].count);
	probes[numOfProbes].randomParticles=(Particle*)malloc(sizeof(Particle)*probes[numOfProbes].count);
	Spawn(probes[numOfProbes].particles,probes[numOfProbes].minPoint,probes[numOfProbes].maxPoint,probes[numOfProbes].maxAge,probes[numOfProbes].count);
	Spawn(probes[numOfProbes].randomParticles,probes[numOfProbes].minPoint,probes[numOfProbes].maxPoint,probes[numOfProbes].maxAge,probes[numOfProbes].count);
	probes[numOfProbes].lines=(Vertex*)calloc(numberOfLines*lengthOfLine,sizeof(Vertex));
	probes[numOfProbes].ribbons=(Vertex*)calloc(numberOfLines*lengthOfLine*2,sizeof(Vertex));
	probes[numOfProbes].streakLines=(Particle*)calloc(numberOfLines*streakLinesLength,sizeof(Particle));
	probes[numOfProbes].seperators=(int*)calloc(numberOfLines,sizeof(int));
	probes[numOfProbes].spawn=(Vertex*)calloc(numberOfLines,sizeof(Vertex));
	SpawnLines(&probes[numOfProbes],1);
	SpawnLines(&probes[numOfProbes],2);
	SpawnStreakLines(&probes[numOfProbes]);
	
	//initAges(&probes[numOfProbes]);
	CreateUnorderedBuffer((ID3D11Device*)pd3dDevice,hr,&probes[numOfProbes]);
	createLinesBuffer((ID3D11Device*)pd3dDevice,&probes[numOfProbes],1);
	createLinesBuffer((ID3D11Device*)pd3dDevice,&probes[numOfProbes],2);
	createStreakLinesBuffer((ID3D11Device*)pd3dDevice,&probes[numOfProbes]);
	selectedIndex++;
	numOfProbes++;
}

//to reload a new volume without restarting the application
void TW_CALL Reload(void* pd3dDevice)
{

	Cleanup();
	OPENFILENAME ofn;
	wchar_t szFile[200];
	ZeroMemory( &ofn , sizeof( ofn));
	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL  ;
	ofn.lpstrFile = szFile ;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( szFile );
	ofn.lpstrFilter = L"All\0*.*\0Text\0*.dat\0";
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL ;
	ofn.nMaxFileTitle = 0 ;
	ofn.lpstrInitialDir=NULL ;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST ;
	GetOpenFileName(&ofn);
	buildTexture((ID3D11Device*)pd3dDevice,ofn);
	gTexture3D->SetResource(shaderResourceViewTexture);
	gTexture3DNext->SetResource(shaderResourceViewTextureNext);
}

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependent on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext )
{
	mousePointsMin.x = 10;
	mousePointsMin.y = 10;
	mousePointsMax.x = 300;
	mousePointsMax.y = 255;
	HRESULT hr;
	std::vector<char> effectData = ReadFileRaw("effect.fxo");
	tfe->onCreateDevice(pd3dDevice);


	V_RETURN(D3DX11CreateEffectFromMemory(effectData.data(), effectData.size(), 0, pd3dDevice, &g_Effect));
	assert(g_Effect && g_Effect->IsValid());
	SAFE_GET_TECHNIQUE(g_Effect,"Render",g_RenderTech);
	SAFE_GET_PASS(g_RenderTech,"SimpleVolumeISO",g_VolumePassISO);
	SAFE_GET_PASS(g_RenderTech,"SimpleVolumeDVR",g_VolumePassDVR);
	SAFE_GET_PASS(g_RenderTech,"Arrows",g_ArrowsPass);
	SAFE_GET_PASS(g_RenderTech,"Particles",g_ParticlesPass);
	SAFE_GET_PASS(g_RenderTech,"Probes",g_ProbesPass);
	SAFE_GET_PASS(g_RenderTech,"VelocityMag",g_Velocity);
	SAFE_GET_PASS(g_RenderTech,"VorticityMag",g_Vorticity);
	SAFE_GET_PASS(g_RenderTech,"Divergence",g_Divergence);
	SAFE_GET_PASS(g_RenderTech,"Q",g_Q);
	SAFE_GET_PASS(g_RenderTech,"StreamLines",g_StreamLines);
	SAFE_GET_PASS(g_RenderTech,"StreakLines",g_StreakLines);
	SAFE_GET_PASS(g_RenderTech,"Integrate",g_Integrate);
	SAFE_GET_PASS(g_RenderTech,"IntegrateRibbons",g_IntegrateRibbons);
	SAFE_GET_PASS(g_RenderTech,"IntegrateStreakLines",g_IntegrateStreakLines);
	SAFE_GET_PASS(g_RenderTech,"Ribbons",g_Ribbons);
	SAFE_GET_PASS(g_RenderTech,"Domain",g_DomainPass);
	SAFE_GET_PASS(g_RenderTech,"Opacity",g_Opacity);
	textureWeight = g_Effect ->GetVariableByName("weight")->AsScalar();
	textureWeightNext = g_Effect ->GetVariableByName("weightNext")->AsScalar();

	OPENFILENAME ofn;
	wchar_t szFile[200];
	ZeroMemory( &ofn , sizeof( ofn));
	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL  ;
	ofn.lpstrFile = szFile ;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( szFile );
	ofn.lpstrFilter = L"Dat Files (*.dat)\0*.dat\0";
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL ;
	ofn.nMaxFileTitle = 0 ;
	ofn.lpstrInitialDir=NULL ;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST ;
	GetOpenFileName(&ofn);
	buildTexture(pd3dDevice,ofn);
	resX=resX*sliceWidth; 
	resY=resY*sliceHeight; 
	resZ=resZ*sliceDepth;
	//setup the camera's position and looking direction
	D3DXVECTOR3 Eye( resX/2, resY/2, (resZ)-20 );
	D3DXVECTOR3 At( resX/2, resY/2, resZ/2 );
	g_Camera->SetViewParams( &Eye, &At );
	f_Camera->SetViewParams(&Eye,&At);



	g_pWorldViewProj = g_Effect->GetVariableByName( "WorldViewProj" )->AsMatrix();
	g_pWorldViewProjInverse = g_Effect->GetVariableByName( "WorldViewProjInverse" )->AsMatrix();
	g_scalingMatrix=g_Effect->GetVariableByName("scalingMatrix")->AsMatrix();
	// Initialize the world matrix
	D3DXMatrixIdentity( &g_World );
	//get shader variables
	LightPosition=g_Effect->GetVariableByName("LightPosition")->AsVector();
	LightDirection=g_Effect->GetVariableByName("LightDirection")->AsVector();
	CameraAt=g_Effect->GetVariableByName("CameraAt")->AsVector();
	g_CamWorldMatrix=g_Effect->GetVariableByName("World")->AsMatrix();
	shininess=g_Effect->GetVariableByName("shine")->AsScalar();
	color=g_Effect->GetVariableByName("color")->AsVector();

	gTexture3D= g_Effect->GetVariableByName("gtexture")->AsShaderResource(); 
	gTexture3D->SetResource(shaderResourceViewTexture);

	gTexture3DNext= g_Effect->GetVariableByName("gtextureNext")->AsShaderResource(); 
	gTexture3DNext->SetResource(shaderResourceViewTextureNext);

	gTexture1D=g_Effect->GetVariableByName("gcolortexture")->AsShaderResource();

	gdispValueMin=g_Effect->GetVariableByName("dispValueMin")->AsScalar();
	gdispValueMax=g_Effect->GetVariableByName("dispValueMax")->AsScalar();
	gstepSize=g_Effect->GetVariableByName("stepSize")->AsScalar();


	gresX=g_Effect->GetVariableByName("resX")->AsScalar();
	gresY=g_Effect->GetVariableByName("resY")->AsScalar();
	gresZ=g_Effect->GetVariableByName("resZ")->AsScalar();
	gpickedZ=g_Effect->GetVariableByName("pickedZ")->AsScalar();
	gresX->SetFloat(resX);
	gresY->SetFloat(resY);
	gresZ->SetFloat(resZ);
	//gpickedZ->SetInt(pickedZ);
	grenderstatic=g_Effect->GetVariableByName("renderstatic")->AsScalar();
	grenderstatic->SetBool(renderStaticVolume);
	//mesh data parsing
	if(meshLoaded)
	{
		ParseData();
		CreateBuffers(pd3dDevice,hr);
		CreateInputLayout(pd3dDevice,hr);
	}
	
		InitGUI(pd3dDevice);
		
		probes=(Probe*)calloc(MAXPROBES,sizeof(Probe));
	

		gparticlesResource=g_Effect->GetVariableByName("gparticlesBuffer")->AsUnorderedAccessView();
	
		gparticlesBufferVariable=g_Effect->GetVariableByName("SpawnBuffer")->AsShaderResource();
	
		grandomParticlesBufferVariable=g_Effect->GetVariableByName("RandomBuffer")->AsShaderResource();

		linesUAVvariable=g_Effect->GetVariableByName("glinesBuffer")->AsUnorderedAccessView();
		linesSRVvariable=g_Effect->GetVariableByName("glinesReadBuffer")->AsShaderResource();
		ribbonsUAVvariable=g_Effect->GetVariableByName("gribbonsBuffer")->AsUnorderedAccessView();
		ribbonsSRVvariable=g_Effect->GetVariableByName("gribbonsReadBuffer")->AsShaderResource();
		StreaklinesUAVvariable=g_Effect->GetVariableByName("gstreakBuffer")->AsUnorderedAccessView();
		StreaklinesSRVvariable=g_Effect->GetVariableByName("gstreakReadBuffer")->AsShaderResource();
		spawnUAVvariable=g_Effect->GetVariableByName("gspawnBuffer")->AsUnorderedAccessView();
		spawnSRVvariable=g_Effect->GetVariableByName("gspawnReadBuffer")->AsShaderResource();
		sepUAVvariable=g_Effect->GetVariableByName("gsepBuffer")->AsUnorderedAccessView();
		sepSRVvariable=g_Effect->GetVariableByName("gsepReadBuffer")->AsShaderResource();
		gseedingInterval=g_Effect->GetVariableByName("seedingInterval")->AsScalar();
		gstreakLinesLength=g_Effect->GetVariableByName("streakLinesLength")->AsScalar();

		gtimestep=g_Effect->GetVariableByName("timeStep")->AsScalar();
		gtimestep->SetFloat(timeStep);
		gsliceThicknessX=g_Effect->GetVariableByName("sliceThicknessX")->AsScalar();
		gsliceThicknessX->SetFloat(sliceWidth);
		gsliceThicknessY=g_Effect->GetVariableByName("sliceThicknessY")->AsScalar();
		gsliceThicknessY->SetFloat(sliceHeight);
		gsliceThicknessZ=g_Effect->GetVariableByName("sliceThicknessZ")->AsScalar();
		gsliceThicknessZ->SetFloat(sliceDepth);
		gmaxAge=g_Effect->GetVariableByName("maxAge")->AsScalar();

		ggridX=g_Effect->GetVariableByName("gridX")->AsScalar();
		ggridY=g_Effect->GetVariableByName("gridY")->AsScalar();
		ggridZ=g_Effect->GetVariableByName("gridZ")->AsScalar();
		ggridX->SetInt(gridX);
		ggridY->SetInt(gridY);
		ggridZ->SetInt(gridZ);
		gflipValue=g_Effect->GetVariableByName("flipValue")->AsScalar();
		gMouseMax=g_Effect->GetVariableByName("mouseMax")->AsVector();
		gMouseMin=g_Effect->GetVariableByName("mouseMin")->AsVector();
		gProbeColor=g_Effect->GetVariableByName("ProbeColor")->AsVector();
		gParticlesColor=g_Effect->GetVariableByName("particlesColor")->AsVector();
		//RWTexture
		gRWTexture3D=g_Effect->GetVariableByName("RWscalarTexture")->AsUnorderedAccessView();
		gRWTexture3D->SetUnorderedAccessView(shaderResourceViewRWTexture);
		gDepthTexture=g_Effect->GetVariableByName("depthTexture")->AsShaderResource();
		gscalarTexture=g_Effect->GetVariableByName("scalarTexture")->AsShaderResource();
		gscalarTexture->SetResource(scalarTextureSVR);

		gintegrationStepSize=g_Effect->GetVariableByName("integrationStepSize")->AsScalar();
		giteration=g_Effect->GetVariableByName("iteration")->AsScalar();
		glengthOfLine=g_Effect->GetVariableByName("lengthOfLine")->AsScalar();
		gnumberOfLines=g_Effect->GetVariableByName("numberOfLines")->AsScalar();
		createIndexBuffer((ID3D11Device*)pd3dDevice);

		gtimeSinceLastFrame=g_Effect->GetVariableByName("timeSinceLastFrame")->AsScalar();
		glifetime=g_Effect->GetVariableByName("lifetime")->AsScalar();

		guseDensity=g_Effect->GetVariableByName("useDensity")->AsScalar();
		guseFade=g_Effect->GetVariableByName("useFade")->AsScalar();
		guseShape=g_Effect->GetVariableByName("useShape")->AsScalar();

		//initial state
		addProbe(pd3dDevice);
		/*addProbe(pd3dDevice);
		probes[1].maxPoint.z+=0.15;
		probes[1].minPoint.z+=0.15;
		probes[1].maxPoint.y+=0.11;
		probes[1].minPoint.y+=0.11;*/
		renderParticles=true;
		renderVorticity=true;
    return S_OK;

}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{

	// Setup the camera's projection parameters
	float fAspectRatio = static_cast<float>(pBackBufferSurfaceDesc->Width) / static_cast<float>(pBackBufferSurfaceDesc->Height);
	g_Camera->SetProjParams( D3DX_PI/4, fAspectRatio, 0.1f, 5000.0f );
	g_Camera->SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);
	g_Camera->SetButtonMasks( MOUSE_MIDDLE_BUTTON, MOUSE_WHEEL, MOUSE_LEFT_BUTTON );
	f_Camera->SetProjParams( D3DX_PI/4, fAspectRatio, 0.1f, 5000.0f );
	tfe->onResizeSwapChain(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

	//create backbuffer mirror texture
	D3D11_TEXTURE2D_DESC* desc=new D3D11_TEXTURE2D_DESC();
	desc->CPUAccessFlags=D3D11_USAGE_DEFAULT;
	desc->BindFlags=D3D11_BIND_SHADER_RESOURCE;
	desc->Format=DXGI_FORMAT_R32_FLOAT;
	desc->Width=pBackBufferSurfaceDesc->Width;
	desc->Height=pBackBufferSurfaceDesc->Height;
	desc->MipLevels=1;
	desc->ArraySize=1;
	desc->SampleDesc.Count=1;
	desc->SampleDesc.Quality=0;
	float* tmp=(float*)calloc(desc->Height*desc->Width,sizeof(float));
	D3D11_SUBRESOURCE_DATA* TextureData= new D3D11_SUBRESOURCE_DATA();
	TextureData->pSysMem=tmp;
	TextureData->SysMemPitch=desc->Width*sizeof(float);
	TextureData->SysMemSlicePitch=(desc->Width*desc->Height)*sizeof(float);
	pd3dDevice->CreateTexture2D(desc,TextureData,&DepthTexture);
		
	D3D11_SHADER_RESOURCE_VIEW_DESC* shaderResourceViewDesc = new D3D11_SHADER_RESOURCE_VIEW_DESC();
	shaderResourceViewDesc->Format=desc->Format;
	shaderResourceViewDesc->ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc->Texture2D.MostDetailedMip=0;
	shaderResourceViewDesc->Texture2D.MipLevels=1;
	pd3dDevice->CreateShaderResourceView(DepthTexture,shaderResourceViewDesc,&DepthSRV); 

	delete shaderResourceViewDesc;
	delete [] tmp;
	delete desc;
	delete TextureData;
	return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	// Update the camera's position based on user input 
	currentStepSize = fElapsedTime;
	
	if(!fps)
		g_Camera->FrameMove( fElapsedTime );
	//FPS camera 
	if(fps)
		f_Camera->FrameMove (fElapsedTime) ;
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
	double fTime, float fElapsedTime, void* pUserContext )
{

	// Clear render target and the depth stencil 
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearRenderTargetView( pRTV, ClearColor );
	pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );
	DWORD dwCurrentTime;
	static bool increasing = true;
	static DWORD dwLastFrameTime = 0;
	static int state = 1;
	if(moving && scaling) 
		scaling=false;	
	
	if(flip) gflipValue->SetFloat(-1);
	else gflipValue->SetFloat(1);

	if(play && !renderStaticVolume)
	{
		dwCurrentTime = timeGetTime();
		//every time step update the current time
		if ((dwCurrentTime - dwLastFrameTime) >= timeStep * 1000) // 1000 miliseconds in a second.
		{
			currentTime = (currentTime + 1)%volumeVars[0];
			switch(state)
			{
				case 1:
					pd3dImmediateContext->UpdateSubresource(Texture3D,0,NULL,scaledValues[currentTime],volumeVars[1]*pointSize*4,volumeVars[1]*pointSize*4*volumeVars[2]);
					state = 2;
					break;

				case 2:
					pd3dImmediateContext->UpdateSubresource(Texture3DNext,0,NULL,scaledValues[currentTime],volumeVars[1]*pointSize*4,volumeVars[1]*pointSize*4*volumeVars[2]);
					state = 1;
					break;			
			}
			//only calculated when a time step has passed
			dwLastFrameTime = dwCurrentTime;		
		}
		switch(state)
		{
		case 1:
			//if Texture3D is the last updated then Texture3DNext's weight will be the times steps since last update and
			// Texture3D's weight will be (fileIndexStep - times steps since last update)
			textureWeightNext->SetFloat((dwCurrentTime - dwLastFrameTime)/1000);
			textureWeight->SetFloat(timeStep - (dwCurrentTime - dwLastFrameTime)/1000);
			break;
		case 2:
			textureWeight->SetFloat((dwCurrentTime - dwLastFrameTime)/1000);
			textureWeightNext->SetFloat(timeStep - (dwCurrentTime - dwLastFrameTime)/1000);
			break;
		}
	}




	if(!fps)
{
		D3DXMatrixMultiply(cameraMatrix,&g_World,g_Camera->GetViewMatrix() );
		D3DXMatrixMultiply(cameraMatrix,cameraMatrix,g_Camera->GetProjMatrix());	
		g_pWorldViewProj->SetMatrix((float*)cameraMatrix);
		D3DXMatrixInverse(cameraMatrixInverse,NULL,cameraMatrix);
		g_pWorldViewProjInverse->SetMatrix((float*)cameraMatrixInverse);
		LightPosition->SetFloatVector((float*)g_Camera->GetEyePt());
		CameraAt->SetFloatVector((float*)g_Camera->GetEyePt());
		LightDirection->SetFloatVector((float*)g_Camera->GetLookAtPt());
		g_CamWorldMatrix->SetMatrix((float*)g_World);
	}
	else
	{
		D3DXMatrixMultiply(cameraMatrix,&g_World,f_Camera->GetViewMatrix() );
		D3DXMatrixMultiply(cameraMatrix,cameraMatrix,f_Camera->GetProjMatrix());	
		g_pWorldViewProj->SetMatrix((float*)cameraMatrix);
		D3DXMatrixInverse(cameraMatrixInverse,NULL,cameraMatrix);
		g_pWorldViewProjInverse->SetMatrix((float*)cameraMatrixInverse);
		LightPosition->SetFloatVector((float*)f_Camera->GetEyePt());
		CameraAt->SetFloatVector((float*)g_Camera->GetEyePt());
		LightDirection->SetFloatVector((float*)f_Camera->GetLookAtPt());
		g_CamWorldMatrix->SetMatrix((float*)g_World);
	}
	//UI to shader values
	gpickedZ->SetInt(pickedZ);
	shininess->SetFloat(shineDisplay*shineFactor);
	colorArray[0]=R;colorArray[1]=G;colorArray[2]=B;
	color->SetFloatVector(colorArray);
	gdispValueMin->SetFloat(dispValueMin);
	gdispValueMax->SetFloat(dispValueMax);
	gstepSize->SetFloat(currentStepSize);
	//transferfunction
	gTexture1D->SetResource(tfe->getSRV());
	//update camera speed values
	f_Camera->SetScalers(cameraRot,cameraTrans);
	//--------------------------------------------------------------------------------------------------

	//Draw the domain volume 
		gresX->SetFloat(resX);
		gresY->SetFloat(resY);
		gresZ->SetFloat(resZ);
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	g_DomainPass->Apply(0, pd3dImmediateContext);
	pd3dImmediateContext->Draw(24,0); 


	//draw probes
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	for(int i=0;i<numOfProbes;i++)
	{
		gMouseMin->SetFloatVector((float*)probes[i].minPoint);
		gMouseMax->SetFloatVector((float*)probes[i].maxPoint);
		if(i==selectedIndex) gProbeColor->SetFloatVector((float*)D3DXVECTOR4(1,1,0,1));
		else gProbeColor->SetFloatVector((float*)D3DXVECTOR4(1,0,0,1));
		g_ProbesPass->Apply(0, pd3dImmediateContext);
		pd3dImmediateContext->Draw(24,0); 

	}

	
		//MESH RENDERING-------------------------------------
	if(renderMesh && meshLoaded)
	{
		pDSV->GetResource(&DepthResource);
		pd3dImmediateContext->CopyResource(DepthTexture,DepthResource);
		gDepthTexture->SetResource(DepthSRV);
		UINT stride = sizeof (Vertex);
		UINT offset = 0;
		pd3dImmediateContext->IASetVertexBuffers (0, 1, &g_VertexBuffer, &stride, &offset);
		pd3dImmediateContext->IASetIndexBuffer (g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		pd3dImmediateContext->IASetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pd3dImmediateContext->IASetInputLayout(g_InputLayout);
		//scaling
		D3DXMatrixScaling(scalingMatrix,1,1,1);
		g_scalingMatrix->SetMatrix((float*)scalingMatrix);
		// Apply pass
		g_MeshPass->Apply(0, pd3dImmediateContext);
		// Draw using index buffer
		pd3dImmediateContext->DrawIndexed(ntriangles*3, 0, 0);
		SAFE_RELEASE(DepthResource);
	}


	if(renderFlow)
	{
		if(renderArrows)
		{
			pd3dImmediateContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
			pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
			ID3D11Buffer* nullBuffer=nullptr;
			UINT zero=0;
			gresX->SetFloat(resX);
			gresY->SetFloat(resY);
			gresZ->SetFloat(resZ);
			pd3dImmediateContext->IASetVertexBuffers(0,1,&nullBuffer,&zero,&zero);
			g_ArrowsPass->Apply(0, pd3dImmediateContext);
			pd3dImmediateContext->Draw(resX*resY*resZ,0);
		}
	
	if(renderParticles)
	{
		for(int i=0;i<numOfProbes;i++)
		{
			gparticlesResource->SetUnorderedAccessView(probes[i].uav);
			gparticlesBufferVariable->SetResource(probes[i].srv);
			grandomParticlesBufferVariable->SetResource(probes[i].randomsrv);
			

			//threads configuration
			if(probes[i].count<=65535*256) 
			{
				gridX=probes[i].count/256;
				if(probes[i].count%256 != 0) gridX++; 
				gridY=1;
				gridZ=1;
			}

		ggridX->SetInt(gridX);
		ggridY->SetInt(gridY);
		ggridZ->SetInt(gridZ);

		gmaxAge->SetFloat(probes[i].maxAge);
		pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		g_ProbesPass->Apply(0, pd3dImmediateContext);
		if(play)
		pd3dImmediateContext->Dispatch(gridX,gridY,gridZ);
		probes[i].ageTrack+=currentStepSize;
		pd3dImmediateContext->CSSetUnorderedAccessViews(0,1,&unorderedNullView,0);
		pd3dImmediateContext->CSSetShaderResources(0,1,&nullView);
		pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		ID3D11Buffer* nullBuffer=nullptr;
		UINT zero=0;
		pd3dImmediateContext->IASetVertexBuffers(0,1,&nullBuffer,&zero,&zero);
		pd3dImmediateContext->VSSetShaderResources(1,1,&probes[i].srv);
		gParticlesColor->SetFloatVector((float*)probes[i].color);
		g_ParticlesPass->Apply(0, pd3dImmediateContext);
		pd3dImmediateContext->Draw(probes[i].count,0);
		
		if(probes[i].ageTrack>=probes[i].maxAge || updateBuffer) 
		{
			Spawn(probes[i].randomParticles,probes[i].minPoint,probes[i].maxPoint,probes[i].maxAge,probes[i].count);
			pd3dImmediateContext->UpdateSubresource(probes[i].randomBuffer,0,nullptr,probes[i].randomParticles,sizeof(Particle)*probes[i].count,1);
			probes[i].ageTrack=minAge;
			if(i==numOfProbes-1)updateBuffer=false;
		}

		}
			

	}

	if(renderStreamLines)
	{
			gintegrationStepSize->SetFloat(integrationStepSize);
			glengthOfLine->SetFloat(lengthOfLine);
			
			for(int i=0;i<numOfProbes;i++)
		{

			//code to update the lines buffer when the number of lines or length change
			if(numberOfLines!=previousNumberOfLines || lengthOfLine != previousLengthOfLine)
			{
				resizeLinesBuffer((ID3D11Device*)pd3dDevice,1);
				previousNumberOfLines=numberOfLines;
				previousLengthOfLine=lengthOfLine;
				updateBuffer=true;
			}
		if(updateBuffer && play)
		{
			SpawnLines(&probes[i],1);
			pd3dImmediateContext->UpdateSubresource(probes[i].linesBuffer,0,nullptr,probes[i].lines,sizeof(Vertex)*numberOfLines*lengthOfLine,1);
			if(i==numOfProbes-1)updateBuffer=false;	
		}

			linesUAVvariable->SetUnorderedAccessView(probes[i].linesUAV);
			linesSRVvariable->SetResource(probes[i].linesSRV);

			//threads configuration
			if(numberOfLines<=65535*256) 
			{
				gridX=numberOfLines/256;
				if(numberOfLines%256 != 0) gridX++; 
				gridY=1;
				gridZ=1;
			}
		
		ggridX->SetInt(gridX);
		ggridY->SetInt(gridY);
		ggridZ->SetInt(gridZ);
		gnumberOfLines->SetInt(numberOfLines);
		g_Integrate->Apply(0,pd3dImmediateContext);
		if(play) pd3dImmediateContext->Dispatch(gridX,gridY,gridZ);
	
		pd3dImmediateContext->CSSetUnorderedAccessViews(0,1,&unorderedNullView,0);
		pd3dImmediateContext->CSSetShaderResources(0,1,&nullView);
		pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
		ID3D11Buffer* nullBuffer=nullptr;
		UINT zero=0;
		pd3dImmediateContext->IASetVertexBuffers(0,1,&nullBuffer,&zero,&zero);
		pd3dImmediateContext->VSSetShaderResources(1,1,&probes[i].linesSRV);
		
	
		for(int i=0;i<numberOfLines;i++)
		{
			giteration->SetInt(i);
			g_StreamLines->Apply(0,pd3dImmediateContext);
			pd3dImmediateContext->Draw(lengthOfLine,0);
		
		}
		}
		
	}


	if(renderRibbons)
	{
			gintegrationStepSize->SetFloat(integrationStepSize);
			glengthOfLine->SetFloat(lengthOfLine*2);

			for(int i=0;i<numOfProbes;i++)
		{

			//code to update the lines buffer when the number of lines or length change or probe moves or scales
			if(numberOfLines!=previousNumberOfLines || lengthOfLine != previousLengthOfLine || ribbonsWidth!=previousRibbonsWidth)
			{
				resizeLinesBuffer((ID3D11Device*)pd3dDevice,2);
				previousNumberOfLines=numberOfLines;
				previousLengthOfLine=lengthOfLine;
				previousRibbonsWidth=ribbonsWidth;
			}
			if(updateBuffer && play)
			{
			SpawnLines(&probes[i],2);
			pd3dImmediateContext->UpdateSubresource(probes[i].ribbonsBuffer,0,nullptr,probes[i].ribbons,sizeof(Vertex)*numberOfLines*lengthOfLine*2,1);
			if(i==numOfProbes-1)updateBuffer=false;	
			}

			ribbonsUAVvariable->SetUnorderedAccessView(probes[i].ribbonsUAV);
			ribbonsSRVvariable->SetResource(probes[i].ribbonsSRV);
			//threads configuration
			if(numberOfLines<=65535*256) 
			{
				gridX=numberOfLines/256;
				if(numberOfLines%256 != 0) gridX++; 
				gridY=1;
				gridZ=1;
			}
		
			ggridX->SetInt(gridX);
			ggridY->SetInt(gridY);
			ggridZ->SetInt(gridZ);
			gnumberOfLines->SetInt(numberOfLines);
			g_IntegrateRibbons->Apply(0,pd3dImmediateContext);
			if(play) pd3dImmediateContext->Dispatch(gridX,gridY,gridZ);
			
			
			pd3dImmediateContext->CSSetUnorderedAccessViews(0,1,&unorderedNullView,0);
			pd3dImmediateContext->CSSetShaderResources(0,1,&nullView);
			pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			ID3D11Buffer* nullBuffer=nullptr;
			UINT zero=0;
			pd3dImmediateContext->IASetVertexBuffers(0,1,&nullBuffer,&zero,&zero);
			pd3dImmediateContext->VSSetShaderResources(1,1,&probes[i].ribbonsSRV);
			
			

			for(int i=0;i<numberOfLines;i++)
			{
			giteration->SetInt(i);
			pd3dImmediateContext->IASetIndexBuffer (indexBuffer, DXGI_FORMAT_R32_UINT, 0);
			g_Ribbons->Apply(0,pd3dImmediateContext);
			pd3dImmediateContext->DrawIndexed(lengthOfLine*2,0,0);
			}

		}
		
	}

	if(renderStreakLines)
	{
			lifetime=seedingInterval*(streakLinesLength-1);
			gintegrationStepSize->SetFloat(integrationStepSize);
			gstreakLinesLength->SetFloat(streakLinesLength);
			gseedingInterval->SetFloat(seedingInterval);
			gnumberOfLines->SetInt(numberOfLines);
			glifetime->SetFloat(lifetime);
			gtimeSinceLastFrame->SetFloat(currentStepSize);
			guseDensity->SetBool(useDensity);
			guseFade->SetBool(useFade);
			guseShape->SetBool(useShape);

			for(int i=0;i<numOfProbes;i++)
			{
				//code to update the streak buffer when the number of lines or length change or probe moves or scales
			if(numberOfLines!=previousNumberOfLines || streakLinesLength != previousstreakLinesLength )
			{
				resizeStreakLinesBuffer((ID3D11Device*)pd3dDevice);
				previousNumberOfLines=numberOfLines;
				previousstreakLinesLength=streakLinesLength;
			}
			if(updateBuffer && play)
			{
			SpawnStreakLines(&probes[i]);
			pd3dImmediateContext->UpdateSubresource(probes[i].streakBuffer,0,nullptr,probes[i].streakLines,sizeof(Particle)*numberOfLines*streakLinesLength,1);
			pd3dImmediateContext->UpdateSubresource(probes[i].spawnBuffer,0,nullptr,probes[i].spawn,sizeof(Vertex)*numberOfLines,1);
			pd3dImmediateContext->UpdateSubresource(probes[i].sepBuffer,0,nullptr,probes[i].seperators,sizeof(int)*numberOfLines,1);
			if(i==numOfProbes-1)updateBuffer=false;	
			}

			StreaklinesUAVvariable->SetUnorderedAccessView(probes[i].streakUAV);
			StreaklinesSRVvariable->SetResource(probes[i].streakSRV);
			spawnUAVvariable->SetUnorderedAccessView(probes[i].spawnUAV);
			spawnSRVvariable->SetResource(probes[i].spawnSRV);
			sepUAVvariable->SetUnorderedAccessView(probes[i].sepUAV);
			sepSRVvariable->SetResource(probes[i].sepSRV);
			//threads configuration (one thread per vertex)
			if(numberOfLines*streakLinesLength<=65535*512) 
			{
				gridX=numberOfLines*streakLinesLength/512;
				if(numberOfLines%512 != 0) gridX++; 
				gridY=1;
				gridZ=1;
			}
			ggridX->SetInt(gridX);
			ggridY->SetInt(gridY);
			ggridZ->SetInt(gridZ);


			
		g_IntegrateStreakLines->Apply(0,pd3dImmediateContext);
		if(play) pd3dImmediateContext->Dispatch(gridX,gridY,gridZ);
		pd3dImmediateContext->CSSetUnorderedAccessViews(0,1,&unorderedNullView,0);
		pd3dImmediateContext->CSSetShaderResources(0,1,&nullView);
		pd3dImmediateContext->CSSetUnorderedAccessViews(1,1,&unorderedNullView,0);
		pd3dImmediateContext->CSSetShaderResources(1,1,&nullView);
		pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		
		g_Opacity->Apply(0,pd3dImmediateContext);
		if(play) pd3dImmediateContext->Dispatch(gridX,gridY,gridZ);
		pd3dImmediateContext->CSSetUnorderedAccessViews(0,1,&unorderedNullView,0);
		pd3dImmediateContext->CSSetShaderResources(0,1,&nullView);
		pd3dImmediateContext->CSSetUnorderedAccessViews(1,1,&unorderedNullView,0);
		pd3dImmediateContext->CSSetShaderResources(1,1,&nullView);
		pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		
		ID3D11Buffer* nullBuffer=nullptr;
		UINT zero=0;
		pd3dImmediateContext->IASetVertexBuffers(0,1,&nullBuffer,&zero,&zero);
		pd3dImmediateContext->VSSetShaderResources(1,1,&probes[i].streakSRV);
		gParticlesColor->SetFloatVector((float*)probes[i].color);
	
		for(int i=0;i<numberOfLines-1;i++)
		{
			giteration->SetInt(i);
			g_StreakLines->Apply(0,pd3dImmediateContext);
			pd3dImmediateContext->Draw(streakLinesLength-1,0);
		
		}

	}


}

	
	//volumes creation
	//threads configuration
	if((VolumeRenderingISO||VolumeRenderingDVR) && (renderVelocity || renderDivergence || renderVorticity || renderQ))
	{
			gridX=resX/sliceWidth/10;
			gridY=resY/sliceHeight/10;
			gridZ=resZ/sliceDepth/10;
			if((int)(resX/sliceWidth)%10!=0) gridX++;
			if((int)(resY/sliceHeight)%10!=0) gridY++;
			if((int)(resZ/sliceDepth)%10!=0) gridZ++;
			ggridX->SetInt(gridX);
			ggridY->SetInt(gridY);
			ggridZ->SetInt(gridZ);
			gresX->SetFloat(resX/sliceWidth);
			gresY->SetFloat(resY/sliceHeight);
			gresZ->SetFloat(resZ/sliceDepth);
			if(renderVelocity) 
				g_Velocity->Apply(0,pd3dImmediateContext);
			if(renderDivergence) 
				g_Divergence->Apply(0,pd3dImmediateContext);
			if(renderVorticity) 
				g_Vorticity->Apply(0,pd3dImmediateContext);
			if(renderQ) 
				g_Q->Apply(0,pd3dImmediateContext);
			if(play)pd3dImmediateContext->Dispatch(gridX,gridY,gridZ);
			pd3dImmediateContext->CSSetUnorderedAccessViews(0,1,&unorderedNullView,0);
			pd3dImmediateContext->CSSetShaderResources(0,1,&nullView);


	gresX->SetFloat(resX);
	gresY->SetFloat(resY);
	gresZ->SetFloat(resZ);
	//Domain rendering (probably good place also for volume rendering)
	pDSV->GetResource(&DepthResource);
	pd3dImmediateContext->CopyResource(DepthTexture,DepthResource);
	gDepthTexture->SetResource(DepthSRV);
	pd3dImmediateContext->VSSetShaderResources(0,1,&scalarTextureSVR);
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	if(VolumeRenderingISO)
		g_VolumePassISO->Apply(0, pd3dImmediateContext);
	else if (VolumeRenderingDVR)
		g_VolumePassDVR->Apply(0, pd3dImmediateContext);
	pd3dImmediateContext->Draw(36,0);
	SAFE_RELEASE(DepthResource);
		}
	

	//GUI
	//tfe->onFrameRender(fTime,fElapsedTime);
	TwDraw();
	
}

}
//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
	SAFE_RELEASE(DepthSRV);
	SAFE_RELEASE(DepthResource);
	SAFE_RELEASE(DepthTexture);
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
	SAFE_RELEASE(g_Effect);
	SAFE_RELEASE(m_vertexBuffer);
	SAFE_RELEASE(m_indexBuffer);
	SAFE_RELEASE(g_VertexBuffer);
	SAFE_RELEASE(g_IndexBuffer);
	SAFE_RELEASE(indexBuffer);
	SAFE_RELEASE(g_InputLayout);
	SAFE_RELEASE(m_InputLayout);
	SAFE_RELEASE(Texture3D);
	SAFE_RELEASE(Texture3DNext);
	SAFE_RELEASE(RWTexture3D);
	SAFE_RELEASE(DepthResource);
	SAFE_RELEASE(DepthTexture);
	SAFE_RELEASE(DepthSRV);
	SAFE_RELEASE(shaderResourceViewTexture);
	SAFE_RELEASE(shaderResourceViewTextureNext);
	SAFE_RELEASE(shaderResourceViewRWTexture);
	SAFE_RELEASE(scalarTextureSVR);
	TwTerminate();
	for(int i=0;i<numOfProbes;i++)
	{
		SAFE_RELEASE(probes[i].srv);
		SAFE_RELEASE(probes[i].uav);
		SAFE_RELEASE(probes[i].randomsrv);
		SAFE_RELEASE(probes[i].buffer);
		SAFE_RELEASE(probes[i].randomBuffer);
		SAFE_RELEASE(probes[i].linesBuffer);
		SAFE_RELEASE(probes[i].ribbonsBuffer);
		SAFE_RELEASE(probes[i].linesSRV);
		SAFE_RELEASE(probes[i].ribbonsSRV);
		SAFE_RELEASE(probes[i].linesUAV);
		SAFE_RELEASE(probes[i].ribbonsUAV);
		SAFE_RELEASE(probes[i].streakUAV);
		SAFE_RELEASE(probes[i].streakSRV);
		SAFE_RELEASE(probes[i].streakBuffer);
		SAFE_RELEASE(probes[i].spawnBuffer);
		SAFE_RELEASE(probes[i].spawnUAV);
		SAFE_RELEASE(probes[i].spawnSRV);
		SAFE_RELEASE(probes[i].sepBuffer);
		SAFE_RELEASE(probes[i].sepUAV);
		SAFE_RELEASE(probes[i].sepSRV);
	}

	for(int i=0;i<volumeVars[0];i++)
	{
		delete scaledValues[i];
	}
	delete scaledValues;
	for(int i=0;i<numOfProbes;i++)
	{
		delete [] probes[i].particles;
		delete [] probes[i].randomParticles;
		delete [] probes[i].lines;
		delete [] probes[i].ribbons;
		delete [] probes[i].streakLines;
		delete [] probes[i].spawn;
		delete [] probes[i].seperators;
	}
	delete [] probes;
	delete cameraMatrix;
	delete cameraMatrixInverse;
	delete scalingMatrix;
	delete [] vertices;
	delete [] indices;
	delete(g_Camera);
	delete(f_Camera);
	tfe->onDestroyDevice();
	delete tfe;

}

//--------------------------------------------------------------------------------------
// Call if device was removed.  Return true to find a new device, false to quit
//--------------------------------------------------------------------------------------
bool CALLBACK OnDeviceRemoved( void* pUserContext )
{
	TwWindowSize(0, 0);
	return true;
}

//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	bool* pbNoFurtherProcessing, void* pUserContext )
{
	//start with processing UI messages 
	switch(uMsg)
	{
		//mouse messages
	case WM_MOUSEMOVE:
		{
			if(leftMouseButtonClicked==0)
			{
				int xdiff= GET_X_LPARAM(lParam)-temp.x;
				int ydiff= GET_Y_LPARAM(lParam)-temp.y;
				if(moving)
				{
				probes[selectedIndex].minPoint.x+=xdiff*0.01;
				probes[selectedIndex].minPoint.y-=ydiff*0.01;
				probes[selectedIndex].maxPoint.x+=xdiff*0.01;
				probes[selectedIndex].maxPoint.y-=ydiff*0.01;
				temp.x=GET_X_LPARAM(lParam);
				temp.y=GET_Y_LPARAM(lParam);
				}

				if(scaling)
				{
				probes[selectedIndex].size.x+=xdiff*0.001;
				probes[selectedIndex].maxPoint.x+=xdiff*0.001;
				probes[selectedIndex].size.y-=ydiff*0.001;
				probes[selectedIndex].maxPoint.y-=ydiff*0.001;
				temp.x=GET_X_LPARAM(lParam);
				temp.y=GET_Y_LPARAM(lParam);
				}
				
			}
			break;
		}
	case WM_LBUTTONUP:
		{
			if(leftMouseButtonClicked==0)
			{
				updateBuffer=true;
			}
		leftMouseButtonClicked=-1;
		
		break;
		}
	}
	if( TwEventWin(hWnd, uMsg, wParam, lParam)) // send event to AntTweakBar
		return 0;
	if(tfe->msgProc(hWnd,uMsg,wParam,lParam)) //tranfer function editor
		return 0;

	if(!fps && leftMouseButtonClicked == -1 && !moving && !scaling)
		g_Camera->HandleMessages( hWnd, uMsg, wParam, lParam );
	if(fps && leftMouseButtonClicked == -1 && !moving && !scaling)//FPS camera
		f_Camera->HandleMessages (hWnd, uMsg, wParam, lParam);

	return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{//FPS camera toggle use SPACE button 
	if( bKeyDown )
	{
		switch( nChar )
		{
		case VK_SPACE:               
			{fps=!fps;return;}
		case 0x5A:
			{
				{changeZ=!changeZ;return;}
			}
		}
	}
}
//helper function
double getDistance(int dX0, int dY0, int dX1, int dY1)
{
    return sqrt((float)(dX1 - dX0)*(dX1 - dX0) + (dY1 - dY0)*(dY1 - dY0));
}
//--------------------------------------------------------------------------------------
// Handle mouse button presses
//--------------------------------------------------------------------------------------
void CALLBACK OnMouse( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
	bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
	int xPos, int yPos, void* pUserContext )
{
	if(selectedIndex<0 || (!moving && !scaling)) {leftMouseButtonClicked = -1;return;}

	if(numOfProbes>0 && selectedIndex>-1)
	{
	if(moving)
	{
		probes[selectedIndex].maxPoint.z+=nMouseWheelDelta*0.001;
		probes[selectedIndex].minPoint.z+=nMouseWheelDelta*0.001;
		updateBuffer=true;
	}
	if(scaling)
	{
		probes[selectedIndex].size.z+=nMouseWheelDelta*0.001;
		probes[selectedIndex].maxPoint.z+=nMouseWheelDelta*0.001;
		updateBuffer=true;
	}
	if(leftMouseButtonClicked == -1 && bLeftButtonDown)
	{	
		leftMouseButtonClicked=0;
		temp.x = xPos;
		temp.y = yPos;
	}

}
}


//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	// Set general DXUT callbacks
	DXUTSetCallbackFrameMove( OnFrameMove );
	DXUTSetCallbackKeyboard( OnKeyboard );
	DXUTSetCallbackMouse( OnMouse );
	DXUTSetCallbackMsgProc( MsgProc );
	DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
	DXUTSetCallbackDeviceRemoved( OnDeviceRemoved );

	// Set the D3D11 DXUT callbacks
	DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
	DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
	DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
	DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
	DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
	DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );

	// Perform any application-level initialization here


	DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
	DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
	DXUTCreateWindow( L"VisTool" );

	// Require 11-level hardware
	DXUTCreateDevice( D3D_FEATURE_LEVEL_11_0, true, 640, 480 );
	DXUTMainLoop(); // Enter into the DXUT render loop

	DXUTShutdown(); //Shuts down DXUT (includes calls to OnD3D11ReleasingSwapChain() and OnD3D11DestroyDevice())
	//	_ASSERTE( _CrtCheckMemory( ) );
	// Perform any application-level cleanup here

	return DXUTGetExitCode();
}

//UI initialization
void InitGUI(ID3D11Device* pd3dDevice)
{
	TwInit(TW_DIRECT3D11, pd3dDevice);
	g_Bar = TwNewBar("Render Control");
	//TwAddVarRW(g_Bar, "Render Flow", TW_TYPE_BOOLCPP, &renderFlow, "");
	TwAddVarRW(g_Bar, "Render Particles", TW_TYPE_BOOLCPP, &renderParticles, "");
	//TwAddVarRW(g_Bar, "Rotation Speed (FPS Cam only)", TW_TYPE_FLOAT, &cameraRot, "min=0.0f max=0.01f step=0.001f");
	//TwAddVarRW(g_Bar, "Movement Speed (FPS Cam only)", TW_TYPE_FLOAT, &cameraTrans, "min=0 max=40");

	TwAddVarRW(g_Bar, "Minimum Displayed Value", TW_TYPE_FLOAT, &dispValueMin," step=0.1f");
	TwAddVarRW(g_Bar, "Maximum Displayed Value", TW_TYPE_FLOAT, &dispValueMax," step=0.1f");

	//TwAddVarRW(g_Bar, "Shininess", TW_TYPE_INT32, &shineDisplay, "min=0 max=25");
	//TwAddVarRW(g_Bar, "Red", TW_TYPE_FLOAT, &R, "min=0 max=1 step=0.01f");
	//TwAddVarRW(g_Bar, "Green", TW_TYPE_FLOAT, &G, "min=0 max=1 step=0.01f");
	//TwAddVarRW(g_Bar, "Blue", TW_TYPE_FLOAT, &B, "min=0 max=1 step=0.01f");
	//TwAddVarRW(g_Bar, "Max Age", TW_TYPE_FLOAT, &maxAge, "min=0 step=10f");
	TwAddVarRW(g_Bar, "Play", TW_TYPE_BOOLCPP, &play, "");
	//TwAddVarRW(g_Bar, "Speed", TW_TYPE_INT32, &speed, "min=1");
	
	char s[15];
	sprintf(s,"min=0 max=%d",volumeVars[0]-1);
	TwAddButton(g_Bar,"Add Probe",addProbe,pd3dDevice,"");
	TwAddButton(g_Bar,"Remove Probe",removeProbe,pd3dDevice,"");
	sprintf(s,"min=0 max=%d",MAXPROBES-1);
	TwAddVarRW(g_Bar, "Selected Probe", TW_TYPE_INT32, &selectedIndex, s);
	TwAddVarRW(g_Bar, "Move", TW_TYPE_BOOLCPP, &moving, "");
	//TwAddVarRW(g_Bar, "Scale", TW_TYPE_BOOLCPP, &scaling, "");
	//TwAddVarRW(g_Bar, "IsoSurface Rendering", TW_TYPE_BOOLCPP, &VolumeRenderingISO, "");
	TwAddVarRW(g_Bar, "DirectVolumeRendering", TW_TYPE_BOOLCPP, &VolumeRenderingDVR, "");
	//TwAddVarRW(g_Bar, "Flip Sign", TW_TYPE_BOOLCPP, &flip, "");
	TwAddVarRW(g_Bar, "Velocity Magnitude", TW_TYPE_BOOLCPP, &renderVelocity, "");
	//TwAddVarRW(g_Bar, "Divergence", TW_TYPE_BOOLCPP, &renderDivergence, "");
	TwAddVarRW(g_Bar, "Vorticity", TW_TYPE_BOOLCPP, &renderVorticity, "");
	TwAddVarRW(g_Bar, "Q parameter", TW_TYPE_BOOLCPP, &renderQ, "");
	//TwAddButton(g_Bar,"Browse",Reload,pd3dDevice,"");
	//TwAddVarRW(g_Bar, "StreamLines", TW_TYPE_BOOLCPP, &renderStreamLines, "");
	//TwAddVarRW(g_Bar, "Lines/Tubes", TW_TYPE_BOOLCPP, &renderStreamTubes, "");
	//TwAddVarRW(g_Bar, "Number of lines", TW_TYPE_INT32, &numberOfLines, "min=1");
	//TwAddVarRW(g_Bar, "Length of line", TW_TYPE_INT32, &lengthOfLine, "min=1 ");
	TwAddVarRW(g_Bar, "Integration step size", TW_TYPE_FLOAT, &integrationStepSize, "min=0 step=0.001f");
	TwAddVarRW(g_Bar, "Ribbons", TW_TYPE_BOOLCPP, &renderRibbons, "");
	TwAddVarRW(g_Bar, "Width", TW_TYPE_FLOAT, &ribbonsWidth, "min=0.0001 step=0.001f");
	TwAddVarRW(g_Bar, "StreakSurfaces", TW_TYPE_BOOLCPP, &renderStreakLines, "");
	//TwAddVarRW(g_Bar, "Lines/Tubes", TW_TYPE_BOOLCPP, &renderStreakTubes, "");
	//TwAddVarRW(g_Bar, "Length of Streaklines", TW_TYPE_INT32, &streakLinesLength, "min=1 ");
	//TwAddVarRW(g_Bar, "Seeding Interval", TW_TYPE_FLOAT, &seedingInterval, "min=0.00001 step=0.0001f");
	TwAddButton(g_Bar,"Restart stream/streak",restart,pd3dDevice,"");
	//TwAddVarRW(g_Bar, "Use Density", TW_TYPE_BOOLCPP, &useDensity, "");
	//TwAddVarRW(g_Bar, "Use Shape", TW_TYPE_BOOLCPP, &useShape, "");
	//TwAddVarRW(g_Bar, "Use Fade", TW_TYPE_BOOLCPP, &useFade, "");
	TwAddButton(g_Bar, "Vertical/Horizontal SpawnLine", switchVerticalHorizontal, pd3dDevice, "");
}
void Spawn(Particle* p,D3DXVECTOR3 min,D3DXVECTOR3 max,float maxage,int count)
{
	srand ( (unsigned) time(NULL) );

	//generate random initial values
	for(int i=0;i<count;i++)

	{	
		float randx=min.x + ((max.x-min.x) * rand()/ (RAND_MAX+1.0));
		float randy=min.y + ((max.y-min.y) * rand()/ (RAND_MAX+1.0));
		float randz=min.z + ((max.z-min.z) * rand()/ (RAND_MAX+1.0));

		p[i].age=minAge+ rand()%(int)maxage;
		p[i].pos=D3DXVECTOR4(randx,randy,randz,1);
		p[i].worldPos = D3DXVECTOR4(0,0,0,1);
		p[i].vec=D3DXVECTOR4(0,0,0,1);
	}
}

void SpawnLines(Probe* probe,int n)
{
	srand ( (unsigned) time(NULL) );
	//1 for lines, 2 for ribbons
	if(n==1)
{
	for(int i=0;i<numberOfLines*lengthOfLine;i++)

	{	
		float randx=probe->minPoint.x + ((probe->maxPoint.x-probe->minPoint.x) * rand()/ (RAND_MAX+1.0));
		float randy=probe->minPoint.y + ((probe->maxPoint.y-probe->minPoint.y) * rand()/ (RAND_MAX+1.0));
		float randz=probe->minPoint.z + ((probe->maxPoint.z-probe->minPoint.z) * rand()/ (RAND_MAX+1.0));
		
		probe->lines[i].Pos=D3DXVECTOR4(randx,randy,randz,1);
	}
}
	if(n==2)
{
		
		
	for(int i=0;i<numberOfLines*lengthOfLine*2;i+=2)

	{	
		float randx=probe->minPoint.x + ((probe->maxPoint.x-probe->minPoint.x) * rand()/ (RAND_MAX+1.0));
		float randy=probe->minPoint.y + ((probe->maxPoint.y-probe->minPoint.y) * rand()/ (RAND_MAX+1.0));
		float randz=probe->minPoint.z + ((probe->maxPoint.z-probe->minPoint.z) * rand()/ (RAND_MAX+1.0));
		
		probe->ribbons[i].Pos=D3DXVECTOR4(randx,randy,randz,1);
		probe->ribbons[i+1].Pos=D3DXVECTOR4(randx,randy+ribbonsWidth,randz,1);
	}	
}

}

void SpawnStreakLines(Probe* probe)
{
	srand ( (unsigned) time(NULL) );
	D3DXVECTOR3 startPoint=probe->minPoint;
	float diff,spacing;
	if(verticalLine){
	diff=probe->maxPoint.y-probe->minPoint.y;
	spacing=diff/numberOfLines;
	for(int i=0;i<numberOfLines;i++)
	{
		for(int j=0;j<streakLinesLength;j++)
			probe->streakLines[i*streakLinesLength+j].pos=D3DXVECTOR4(startPoint.x,startPoint.y,startPoint.z,1)+D3DXVECTOR4(0,i*spacing,0,0);
		
	}
	}
	else
	{
	diff=probe->maxPoint.z-probe->minPoint.z;
	spacing=diff/numberOfLines;
	for(int i=0;i<numberOfLines;i++)
	{
		for(int j=0;j<streakLinesLength;j++)
			probe->streakLines[i*streakLinesLength+j].pos=D3DXVECTOR4(startPoint.x,startPoint.y,startPoint.z,1)+D3DXVECTOR4(0,0,i*spacing,0);
		
	}
	}

	initAges(probe);
	initSeperators(probe);
}

HRESULT CreateUnorderedBuffer(ID3D11Device* pd3dDevice,HRESULT hr,Probe* p)
{
	D3D11_BUFFER_DESC* bd = new D3D11_BUFFER_DESC(); // init to binary zero = default values
	bd->Usage = D3D11_USAGE_DEFAULT;
	bd->ByteWidth = sizeof (Particle) * p->count; // size in bytes
	bd->BindFlags = D3D11_BIND_UNORDERED_ACCESS| D3D11_BIND_SHADER_RESOURCE;
	bd->MiscFlags= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	// Define initial data
	D3D11_SUBRESOURCE_DATA* ParticlesData = new D3D11_SUBRESOURCE_DATA();
	ParticlesData->pSysMem = p->particles; // pointer to the array
	bd->StructureByteStride=sizeof(Particle);
	//create buffer
	V_RETURN( pd3dDevice->CreateBuffer (bd, ParticlesData, &p->buffer) );

	//create unordered access view
	D3D11_UNORDERED_ACCESS_VIEW_DESC* vd=new D3D11_UNORDERED_ACCESS_VIEW_DESC();
	vd->Format=DXGI_FORMAT_UNKNOWN;
	vd->Buffer.NumElements=p->count;
	//vd->Buffer.Flags=D3D11_BUFFER_UAV_FLAG_COUNTER;
	pd3dDevice->CreateUnorderedAccessView(p->buffer,NULL,&p->uav);


	//create another view for reading
	D3D11_SHADER_RESOURCE_VIEW_DESC* sd=new D3D11_SHADER_RESOURCE_VIEW_DESC();
	sd->ViewDimension=D3D11_SRV_DIMENSION_BUFFER;
	D3D11_BUFFER_SRV bufferSpecifics;
	bufferSpecifics.ElementOffset=0;
	bufferSpecifics.FirstElement=0;
	bufferSpecifics.ElementWidth=sizeof(Particle);
	bufferSpecifics.NumElements=p->count;
	sd->Buffer=bufferSpecifics;
	sd->Format=DXGI_FORMAT_UNKNOWN;
	pd3dDevice->CreateShaderResourceView(p->buffer,sd,&p->srv);

	//create another buffer for random Values
	// Define initial data
	D3D11_SUBRESOURCE_DATA* RandomParticlesData = new D3D11_SUBRESOURCE_DATA();
	RandomParticlesData->pSysMem = p->randomParticles; // pointer to the array
	bd->StructureByteStride=sizeof(Particle);
	//create buffer 
	V_RETURN( pd3dDevice->CreateBuffer (bd, RandomParticlesData, &p->randomBuffer) );
	pd3dDevice->CreateShaderResourceView(p->randomBuffer,sd,&p->randomsrv);

	delete bd;
	delete vd;
	delete sd;
	delete ParticlesData;
	delete RandomParticlesData;
	return hr;
}

void createLinesBuffer(ID3D11Device* pd3dDevice,Probe* p,int n)
{
	D3D11_BUFFER_DESC* bd = new D3D11_BUFFER_DESC(); // init to binary zero = default values
	bd->Usage = D3D11_USAGE_DEFAULT;
	bd->ByteWidth = sizeof (Vertex) * numberOfLines*lengthOfLine*n; // size in bytes
	bd->BindFlags = D3D11_BIND_UNORDERED_ACCESS| D3D11_BIND_SHADER_RESOURCE;
	bd->MiscFlags= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	D3D11_SUBRESOURCE_DATA* LinesData = new D3D11_SUBRESOURCE_DATA();
	if(n==1)LinesData->pSysMem = p->lines;
	if(n==2)LinesData->pSysMem=p->ribbons;
	bd->StructureByteStride=sizeof(Vertex);
	//create buffer
	if(n==1)pd3dDevice->CreateBuffer (bd, LinesData, &p->linesBuffer);
	if(n==2)pd3dDevice->CreateBuffer (bd, LinesData, &p->ribbonsBuffer);

	//create unordered access view
	D3D11_UNORDERED_ACCESS_VIEW_DESC* vd=new D3D11_UNORDERED_ACCESS_VIEW_DESC();
	vd->Format=DXGI_FORMAT_UNKNOWN;
	vd->Buffer.NumElements=numberOfLines*lengthOfLine*n;
	if(n==1)pd3dDevice->CreateUnorderedAccessView(p->linesBuffer,NULL,&p->linesUAV);
	if(n==2)pd3dDevice->CreateUnorderedAccessView(p->ribbonsBuffer,NULL,&p->ribbonsUAV);

	//create another view for reading
	D3D11_SHADER_RESOURCE_VIEW_DESC* sd=new D3D11_SHADER_RESOURCE_VIEW_DESC();
	sd->ViewDimension=D3D11_SRV_DIMENSION_BUFFER;
	D3D11_BUFFER_SRV bufferSpecifics;
	bufferSpecifics.ElementOffset=0;
	bufferSpecifics.FirstElement=0;
	bufferSpecifics.ElementWidth=sizeof(Vertex);
	bufferSpecifics.NumElements=numberOfLines*lengthOfLine*n;
	sd->Buffer=bufferSpecifics;
	sd->Format=DXGI_FORMAT_UNKNOWN;
	if(n==1)pd3dDevice->CreateShaderResourceView(p->linesBuffer,sd,&p->linesSRV);
	if(n==2)pd3dDevice->CreateShaderResourceView(p->ribbonsBuffer,sd,&p->ribbonsSRV);

	delete bd;
	delete vd;
	delete sd;
	delete LinesData;

}

void createStreakLinesBuffer(ID3D11Device* pd3dDevice,Probe* p)
{
	D3D11_BUFFER_DESC* bd = new D3D11_BUFFER_DESC(); // init to binary zero = default values
	bd->Usage = D3D11_USAGE_DEFAULT;
	bd->ByteWidth = sizeof (Particle) * numberOfLines*streakLinesLength; // size in bytes
	bd->BindFlags = D3D11_BIND_UNORDERED_ACCESS| D3D11_BIND_SHADER_RESOURCE;
	bd->MiscFlags= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	D3D11_SUBRESOURCE_DATA* LinesData = new D3D11_SUBRESOURCE_DATA();
	LinesData->pSysMem=p->streakLines;
	bd->StructureByteStride=sizeof(Particle);
	pd3dDevice->CreateBuffer(bd,LinesData,&p->streakBuffer);

	//create unordered access view
	D3D11_UNORDERED_ACCESS_VIEW_DESC* vd=new D3D11_UNORDERED_ACCESS_VIEW_DESC();
	vd->Format=DXGI_FORMAT_UNKNOWN;
	vd->Buffer.NumElements=numberOfLines*streakLinesLength;
	pd3dDevice->CreateUnorderedAccessView(p->streakBuffer,NULL,&p->streakUAV);

	
	//create another view for reading
	D3D11_SHADER_RESOURCE_VIEW_DESC* sd=new D3D11_SHADER_RESOURCE_VIEW_DESC();
	sd->ViewDimension=D3D11_SRV_DIMENSION_BUFFER;
	D3D11_BUFFER_SRV bufferSpecifics;
	bufferSpecifics.ElementOffset=0;
	bufferSpecifics.FirstElement=0;
	bufferSpecifics.ElementWidth=sizeof(Particle);
	bufferSpecifics.NumElements=numberOfLines*streakLinesLength;
	sd->Buffer=bufferSpecifics;
	sd->Format=DXGI_FORMAT_UNKNOWN;
	pd3dDevice->CreateShaderResourceView(p->streakBuffer,sd,&p->streakSRV);

	delete bd;
	delete vd;
	delete sd;
	delete LinesData;

	//create spawn locatins buffer
	
	D3D11_BUFFER_DESC* bd2 = new D3D11_BUFFER_DESC(); // init to binary zero = default values
	bd2->Usage = D3D11_USAGE_DEFAULT;
	bd2->ByteWidth = sizeof (Vertex) * numberOfLines; // size in bytes
	bd2->BindFlags = D3D11_BIND_UNORDERED_ACCESS| D3D11_BIND_SHADER_RESOURCE;
	bd2->MiscFlags= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	D3D11_SUBRESOURCE_DATA* LinesData2 = new D3D11_SUBRESOURCE_DATA();
	LinesData2->pSysMem=p->spawn;
	bd2->StructureByteStride=sizeof(Vertex);
	pd3dDevice->CreateBuffer(bd2,LinesData2,&p->spawnBuffer);

	//create unordered access view
	D3D11_UNORDERED_ACCESS_VIEW_DESC* vd2=new D3D11_UNORDERED_ACCESS_VIEW_DESC();
	vd2->Format=DXGI_FORMAT_UNKNOWN;
	vd2->Buffer.NumElements=numberOfLines;
	pd3dDevice->CreateUnorderedAccessView(p->spawnBuffer,NULL,&p->spawnUAV);

	//create another view for reading
	D3D11_SHADER_RESOURCE_VIEW_DESC* sd2=new D3D11_SHADER_RESOURCE_VIEW_DESC();
	sd2->ViewDimension=D3D11_SRV_DIMENSION_BUFFER;
	D3D11_BUFFER_SRV bufferSpecifics2;
	bufferSpecifics2.ElementOffset=0;
	bufferSpecifics2.FirstElement=0;
	bufferSpecifics2.ElementWidth=sizeof(Vertex);
	bufferSpecifics2.NumElements=numberOfLines;
	sd2->Buffer=bufferSpecifics2;
	sd2->Format=DXGI_FORMAT_UNKNOWN;
	pd3dDevice->CreateShaderResourceView(p->spawnBuffer,sd2,&p->spawnSRV);

	delete bd2;
	delete vd2;
	delete sd2;
	delete LinesData2;

	//create seperators buffer n views
	
	D3D11_BUFFER_DESC* bd3 = new D3D11_BUFFER_DESC(); // init to binary zero = default values
	bd3->Usage = D3D11_USAGE_DEFAULT;
	bd3->ByteWidth = sizeof (int) * numberOfLines; // size in bytes
	bd3->BindFlags = D3D11_BIND_UNORDERED_ACCESS| D3D11_BIND_SHADER_RESOURCE;
	bd3->MiscFlags= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	D3D11_SUBRESOURCE_DATA* LinesData3 = new D3D11_SUBRESOURCE_DATA();
	LinesData3->pSysMem=p->seperators;
	bd3->StructureByteStride=sizeof(int);
	pd3dDevice->CreateBuffer(bd3,LinesData3,&p->sepBuffer);

	//create unordered access view
	D3D11_UNORDERED_ACCESS_VIEW_DESC* vd3=new D3D11_UNORDERED_ACCESS_VIEW_DESC();
	vd3->Format=DXGI_FORMAT_UNKNOWN;
	vd3->Buffer.NumElements=numberOfLines;
	pd3dDevice->CreateUnorderedAccessView(p->sepBuffer,NULL,&p->sepUAV);

	//create another view for reading
	D3D11_SHADER_RESOURCE_VIEW_DESC* sd3=new D3D11_SHADER_RESOURCE_VIEW_DESC();
	sd3->ViewDimension=D3D11_SRV_DIMENSION_BUFFER;
	D3D11_BUFFER_SRV bufferSpecifics3;
	bufferSpecifics3.ElementOffset=0;
	bufferSpecifics3.FirstElement=0;
	bufferSpecifics3.ElementWidth=sizeof(int);
	bufferSpecifics3.NumElements=numberOfLines;
	sd3->Buffer=bufferSpecifics3;
	sd3->Format=DXGI_FORMAT_UNKNOWN;
	pd3dDevice->CreateShaderResourceView(p->sepBuffer,sd3,&p->sepSRV);

	delete bd3;
	delete vd3;
	delete sd3;
	delete LinesData3;
}

void initSeperators(Probe* probe)
{
	for(int i=0;i<numberOfLines;i++)
	{
		probe->seperators[i]=0;
	}
}


void initAges(Probe* probe)
{
	for(int i=0;i<numberOfLines;i++)
	{
		for(int j=0;j<streakLinesLength;j++)
		{
			probe->streakLines[i*streakLinesLength+j].age=0-j*seedingInterval;
		}
		probe->spawn[i].Pos=probe->streakLines[i*streakLinesLength].pos;
	}
}

void createIndexBuffer(ID3D11Device* pd3dDevice)
{
	int* indices=(int*)calloc((lengthOfLine*2/4)*6,sizeof(int));
	int k=0;
	for(int i=0;i<((lengthOfLine*2/4)*6)-5;i+=6)
	{
		indices[i]=k;
		indices[i+1]=k+1;
		indices[i+2]=k+2;
		indices[i+3]=k+2;
		indices[i+4]=k+3;
		indices[i+5]=k+1;
		//indices[i+6]=k+2;
		k+=2;
	}
	D3D11_BUFFER_DESC* bd = new D3D11_BUFFER_DESC(); // init to binary zero = default values
	bd->Usage = D3D11_USAGE_DEFAULT;
	bd->ByteWidth = sizeof (int) * lengthOfLine*2; // size in bytes
	bd->BindFlags = D3D11_BIND_INDEX_BUFFER ;

	D3D11_SUBRESOURCE_DATA* IndexData = new D3D11_SUBRESOURCE_DATA();
	IndexData->pSysMem=indices;
	bd->StructureByteStride=sizeof(int);
	//create buffer
	pd3dDevice->CreateBuffer (bd, IndexData, &indexBuffer);

	delete [] indices;
	delete bd;
	delete IndexData;
}

void resizeLinesBuffer(ID3D11Device* pd3dDevice,int n)
{
	for(int i=0;i<numOfProbes;i++)
	{
		if(n==1){
		free(probes[i].lines);
		probes[i].lines=(Vertex*)calloc(numberOfLines*lengthOfLine,sizeof(Vertex));
		}
		if(n==2){
		free(probes[i].ribbons);
		probes[i].ribbons=(Vertex*)calloc(numberOfLines*lengthOfLine*2,sizeof(Vertex));
		}
		SpawnLines(&probes[i],n);
		if(n==1){
		SAFE_RELEASE(probes[i].linesSRV);
		SAFE_RELEASE(probes[i].linesUAV);
		SAFE_RELEASE(probes[i].linesBuffer);
		}
		if(n==2)
		{
		SAFE_RELEASE(probes[i].ribbonsSRV);
		SAFE_RELEASE(probes[i].ribbonsUAV);
		SAFE_RELEASE(probes[i].ribbonsBuffer);
		}
		createLinesBuffer(pd3dDevice,&probes[i],n);

	}

}
void resizeStreakLinesBuffer(ID3D11Device* pd3dDevice)
{
	for(int i=0;i<numOfProbes;i++)
	{
		free(probes[i].streakLines);
		free(probes[i].spawn);
		free(probes[i].seperators);
		probes[i].streakLines=(Particle*)calloc(numberOfLines*streakLinesLength,sizeof(Particle));
		probes[i].spawn=(Vertex*)calloc(numberOfLines,sizeof(Vertex));
		probes[i].seperators=(int*)calloc(numberOfLines,sizeof(int));
		SpawnStreakLines(&probes[i]);
		//initAges(&probes[i]);
		SAFE_RELEASE(probes[i].streakSRV);
		SAFE_RELEASE(probes[i].streakUAV);
		SAFE_RELEASE(probes[i].streakBuffer);
		SAFE_RELEASE(probes[i].spawnSRV);
		SAFE_RELEASE(probes[i].spawnUAV);
		SAFE_RELEASE(probes[i].spawnBuffer);
		SAFE_RELEASE(probes[i].sepSRV);
		SAFE_RELEASE(probes[i].sepUAV);
		SAFE_RELEASE(probes[i].sepBuffer);
		createStreakLinesBuffer(pd3dDevice,&probes[i]);
	}
}

int ParseData()
{
	p_ply ply = ply_open(meshFilePathName.c_str(), NULL, 0, NULL);
	if (!ply) return 1;
	if (!ply_read_header(ply)) return 1;
	nvertices = ply_set_read_cb(ply, "vertex", "x", vertex_cb, NULL, 0);
	ply_set_read_cb(ply, "vertex", "y", vertex_cb, NULL, 0);
	ply_set_read_cb(ply, "vertex", "z", vertex_cb, NULL, 0);
	ply_set_read_cb(ply, "vertex", "nx", vertex_cb, NULL, 0);
	ply_set_read_cb(ply, "vertex", "ny", vertex_cb, NULL, 0);
	ply_set_read_cb(ply, "vertex", "nz", vertex_cb, NULL, 1);
	ntriangles = ply_set_read_cb(ply, "face", "vertex_indices", face_cb, NULL, 0);
	printf("%ld\n%ld\n", nvertices, ntriangles);
	vertices=(Vertex*)calloc(nvertices,sizeof(Vertex));
	indices=(int*)calloc(3*ntriangles,sizeof(int));
	if (!ply_read(ply)) return 1;
	ply_close(ply);
	return 1;
}

HRESULT CreateBuffers(ID3D11Device* pd3dDevice,HRESULT hr)
{
	D3D11_BUFFER_DESC* bd = new D3D11_BUFFER_DESC(); // init to binary zero = default values
	bd->Usage = D3D11_USAGE_DEFAULT;
	bd->ByteWidth = sizeof (Vertex) * nvertices; // size in bytes
	bd->BindFlags = D3D11_BIND_VERTEX_BUFFER;
	// Define initial data
	D3D11_SUBRESOURCE_DATA* VerticesInitData = new D3D11_SUBRESOURCE_DATA();
	VerticesInitData->pSysMem = vertices; // pointer to the array
	//create buffer
	V_RETURN( pd3dDevice->CreateBuffer (bd, VerticesInitData, &g_VertexBuffer) );
	//index buffer
	D3D11_BUFFER_DESC* bd2 = new D3D11_BUFFER_DESC(); // init to binary zero = default values
	//bd2={0};
	bd2->Usage = D3D11_USAGE_DEFAULT;
	bd2->ByteWidth = sizeof (int) * ntriangles * 3; // size in bytes
	bd2->BindFlags = D3D11_BIND_INDEX_BUFFER;
	// Define initial data
	D3D11_SUBRESOURCE_DATA* IndicesInitData =new D3D11_SUBRESOURCE_DATA();
	IndicesInitData->pSysMem = indices; // pointer to the array
	V_RETURN( pd3dDevice->CreateBuffer (bd2, IndicesInitData, &g_IndexBuffer) );
	delete bd2;
	delete bd;
	delete VerticesInitData;
	delete IndicesInitData;
	return hr;
}

HRESULT CreateInputLayout(ID3D11Device* pd3dDevice,HRESULT hr)
{
	auto AAE = D3D11_APPEND_ALIGNED_ELEMENT;
	auto IPVD = D3D11_INPUT_PER_VERTEX_DATA;
	// Array of descriptions for each vertex attribute
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, AAE, IPVD, 0 },
		{ "NORMAL" , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, AAE, IPVD, 0 },
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);
	// Get input signature of pass using this layout
	D3DX11_PASS_DESC* PassDesc = new D3DX11_PASS_DESC();
	g_RenderTech->GetPassByName("MeshPass")->GetDesc( PassDesc );
	SAFE_GET_PASS(g_RenderTech,"MeshPass",g_MeshPass);
	// Create the input layout
	V_RETURN( pd3dDevice->CreateInputLayout (layout, numElements,PassDesc->pIAInputSignature, PassDesc->IAInputSignatureSize, &g_InputLayout));
	delete PassDesc;
	return hr;
}

