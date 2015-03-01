//structures
struct Vertex {
    float4 pos : POSITION;
    float3 nor : NORMAL;
}

;
struct ParticleDepth
{
    float4 color : SV_Target;
    float depth : SV_Depth;
}

;
struct Particle{
    float4 pos : SV_Position;
    float4 vec : NORMAL;
    float4 worldPos : WorldPosition;
    float age : AGE;
}

;
struct PSIn {
    float4 pos : SV_Position;
    float3 nor : NORMAL;
    float4 ospos : TEXTCOORD0;
    float3 lightDir : lightdirection;
	float3 nextVertix : nextvertix;
	float3 lightDirNext : lightdirectionnext;
	float4 color : COLOR0;
};

//struct for the straight line equation in the form of X = X0 + (X1-X0)t where X0 is startpoint and X1-X0 is the direction
struct lightRay
{
    float3 startpoint;
    float3 secondpoint;
    float3 direction;
}

;

struct CylinderSpherePSIn
{
	float4 pos : SV_Position;
    float4 vec : NORMAL;
    float4 worldPos : WorldPosition;
	int type : pixeltype;
	float3 lightDir : lightdirection;
	lightRay axis : cylinderaxis;
	float3 origin : sphereorigin;
}
;
struct float3float{
    float3 normal;
    float c;
	float depth;
}

;
//Corresponds to D3D11_RASTERIZER_DESC
RasterizerState rsCullNone
{
    CullMode = None;
    FillMode=Solid;
}

;
RasterizerState rsCullFront {
    CullMode = Front;
    // can be Front | Back | None
	FillMode=Solid;
}

;
RasterizerState rsCullBack {
    CullMode = Back;
    FillMode=Solid;
}

;
//Corresponds to D3D11_DEPTH_STENCIL_DESC
DepthStencilState dsEnableDepth
{
    DepthEnable = true;
    DepthFunc = Less;
}

;
DepthStencilState dsDisableDepth
{
    DepthEnable = false;
    DepthFunc = Less;
    DepthWriteMask = 0;
}

;
DepthStencilState dsEnableDepthDisableWrite
{
    DepthEnable = true;
    DepthFunc = Less;
	DepthWriteMask = 0;
}

;
SamplerState samLinear{
	Filter=MIN_MAG_MIP_LINEAR;
	AddressU = Clamp;
	AddressV = Clamp;
	AddressW = Clamp;
};

;
//Corresponds to D3D11_BLEND_DESC
BlendState bsBlending
{
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = ADD;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
}

;
BlendState bsNoBlending
{
    BlendEnable[0] = false;
}

;
//buffer 
cbuffer cbChangesEveryFrame
{
    matrix WorldViewProj;
    matrix WorldViewProjInverse;
    matrix WorldView;
    matrix WorldViewPInverse;
    matrix World;
    matrix scalingMatrix;
    float3 LightPosition;
    float3 LightDirection;
    float3 CameraAt;
    float shine;
    float4 ProbeColor;
    float4 particlesColor;
    int ProbeMaxAge;
    float dispValueMin;
    float dispValueMax;
    float stepSize;
    int pickedZ;
	int iteration;
	int lengthOfLine;
	int numberOfLines;
    float3 mouseMin,mouseMax;
	float integrationStepSize;
	float seedingInterval;
	int streakLinesLength;
	int lifetime;
	float timeSinceLastFrame;
	bool useFade,useDensity,useShape;
}

;
float weight, weightNext,timeStep,sliceThicknessX,sliceThicknessY,sliceThicknessZ;
Texture3D<float4> gtexture;
Texture3D<float4> gtextureNext;
//the 2nd texture
Texture1D<float4> gcolortexture;
//texture representing the color transfer function
RWTexture3D<float> RWscalarTexture;
//the writable volume texture
Texture3D<float> scalarTexture;
//read only handle to the scalar texture for ray tracing
Texture2D<float> depthTexture;
int gridX,gridY,gridZ,flipValue;
float resX,resY,resZ,maxAge;
bool renderstatic;
RWStructuredBuffer<Particle> gparticlesBuffer;
StructuredBuffer<Particle> SpawnBuffer;
StructuredBuffer<Particle> RandomBuffer;
RWStructuredBuffer<Vertex> glinesBuffer;
StructuredBuffer<Vertex> glinesReadBuffer;
RWStructuredBuffer<Vertex> gribbonsBuffer;
StructuredBuffer<Vertex> gribbonsReadBuffer;
RWStructuredBuffer<Particle> gstreakBuffer;
StructuredBuffer<Particle> gstreakReadBuffer;
RWStructuredBuffer<Vertex> gspawnBuffer;
StructuredBuffer<Vertex> gspawnReadBuffer;
RWStructuredBuffer<int> gsepBuffer;
StructuredBuffer<int> gsepReadBuffer;

float4 Lighting(PSIn i,float3 normal, float4 diffuse)
{
	i.lightDir=normalize(mul(LightPosition,World)-mul(i.ospos,World));
	float4 colorOutput={0,0,0,1};
	float4 ambient={0.1,0.1,0.1,1};
	float ambientIntesity=1;
	float diffuseIntensity=4;
	float NdotL=saturate(dot(normal,i.lightDir));	
	float3 Reflect = normalize(2 *normal * NdotL + i.lightDir); 
	float4 specular = pow(saturate(dot(Reflect, i.lightDir)),100);
	colorOutput+=ambient*ambientIntesity+(diffuse*NdotL*diffuseIntensity)+specular;
return colorOutput;
}

PSIn DomainVS (uint id : SV_VertexID)  
{
    PSIn result;
    result.nor=0;
    result.lightDir=float3(0,0,0);
	result.nextVertix = float3(0,0,0);
	result.lightDirNext = float3(0,0,0);
	result.color=float4(0,0,0,0);
    switch(id) {
        default:
		case 0: {
            result.pos= mul(float4( 0, resY, 0, 1.0),WorldViewProj);
            result.ospos=float4( 0, resY, 0, 1.0) ;
            return result;
        }


		case 1:{
            result.pos=mul(float4(resX, resY, 0, 1.0),WorldViewProj)	;
            result.ospos=float4(resX, resY, 0, 1.0) ;
            return result;
        }


		case 2: {
            result.pos=mul(float4( 0, resY, 0, 1.0),WorldViewProj);
            result.ospos=float4( 0, resY, 0, 1.0) ;
            return result;
        }


		case 3:{
            result.pos=mul(float4( 0, 0, 0, 1.0),WorldViewProj)	;
            result.ospos=float4( 0, 0, 0, 1.0) ;
            return result;
        }


		case 4:{
            result.pos=mul(float4(0, resY, 0, 1.0),WorldViewProj)	;
            result.ospos=float4(0, resY, 0, 1.0) ;
            return result;
        }


		case 5: {
            result.pos=mul(float4(0, resY, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4(0, resY, resZ, 1.0) ;
            return result;
        }


		case 6: {
            result.pos=mul(float4(resX, resY, 0, 1.0),WorldViewProj)	;
            result.ospos=float4(resX, resY, 0, 1.0) ;
            return result;
        }


		case 7: {
            result.pos=mul(float4(resX, resY, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4(resX, resY, resZ, 1.0) ;
            return result;
        }


		case 8: {
            result.pos=mul(float4(resX, resY, 0, 1.0),WorldViewProj)	;
            result.ospos=float4(resX, resY, 0, 1.0) ;
            return result;
        }


		case 9: {
            result.pos=mul(float4(resX, 0, 0, 1.0),WorldViewProj)	;
            result.ospos=float4(resX, 0, 0, 1.0) ;
            return result;
        }


				//
		case 10: {
            result.pos=mul(float4(resX, resY, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4(resX, resY, resZ, 1.0) ;
            return result;
        }


		case 11:{
            result.pos=mul(float4(resX, 0, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4(resX, 0, resZ, 1.0) ;
            return result;
        }


		case 12: {
            result.pos=mul(float4(resX, resY, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4(resX, resY, resZ, 1.0) ;
            return result;
        }


		case 13:{
            result.pos=mul(float4(  0, resY, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4(  0, resY, resZ, 1.0) ;
            return result;
        }


				//
		case 14: {
            result.pos=mul(float4(resX, 0, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4(resX, 0, resZ, 1.0) ;
            return result;
        }


		case 15: {
            result.pos=mul(float4(resX, 0, 0, 1.0),WorldViewProj)	;
            result.ospos=float4(resX, 0, 0, 1.0);
            return result;
        }


				 //
		case 16:{
            result.pos=mul(float4( 0, resY, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4( 0, resY, resZ, 1.0) ;
            return result;
        }


		case 17: {
            result.pos=mul(float4( 0, 0, resZ, 1.0),WorldViewProj);
            result.ospos=float4( 0, 0, resZ, 1.0) ;
            return result;
        }


				 //
		case 18: {
            result.pos=mul(float4( 0, 0, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4( 0, 0, resZ, 1.0) ;
            return result;
        }


		case 19:{
            result.pos=mul(float4(0, 0, 0, 1.0),WorldViewProj)	;
            result.ospos=float4(0, 0, 0, 1.0);
            return result;
        }


		case 20:{
            result.pos=mul(float4( 0, 0, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4( 0, 0, resZ, 1.0) ;
            return result;
        }


		case 21: {
            result.pos=mul(float4( resX, 0, resZ, 1.0),WorldViewProj);
            result.ospos=float4( resX, 0, resZ, 1.0) ;
            return result;
        }


				 //
		case 22: {
            result.pos=mul(float4( 0, 0, 0, 1.0),WorldViewProj);
            result.ospos=float4( 0, 0, 0, 1.0) ;
            return result;
        }


		case 23: {
            result.pos=mul(float4( resX, 0, 0, 1.0),WorldViewProj);
            result.ospos=float4( resX, 0, 0, 1.0) ;
            return result;
        }


		
    }
}


ParticleDepth DomainPS(PSIn i) 
{
	float4 pixelPos = mul(i.ospos,WorldViewProj);
	ParticleDepth resultParticle;
	resultParticle.depth = pixelPos.z/pixelPos.w;
	resultParticle.color = float4(1,0,0,1);
    return resultParticle;
}




//**************************************************************************************************//
//function gets the first intersection of line and cube (the point is the entry point) //
lightRay GetIntersection (lightRay myRay)
{
    // t is the coeffecient of the equation
	//myX,myY and myZ are the variables of the equation
	float t,myX,myY,myZ;
    //the two intersection points (entry and exit)
	float3 point1,point2;
    point1 = float3(myRay.startpoint.x, myRay.startpoint.y, myRay.startpoint.z);
    point2 = float3(myRay.startpoint.x, myRay.startpoint.y, myRay.startpoint.z);
    bool firstPointFound = false;
	bool secondPointFound = false;


	//check intersection for the plane of X = 1 if the startpoint is not in this plane
	if((myRay.startpoint.x != resX) && (myRay.direction.x!=0))
	{
        //get value of t for the point of the line that is on the plane X = 1
		t = (resX - myRay.startpoint.x) / myRay.direction.x;
        myY = myRay.startpoint.y + myRay.direction.y * t;
        myZ = myRay.startpoint.z + myRay.direction.z * t;
        if ((resY >= myY)&& (myY >= 0)&&(resZ >= myZ)&& (myZ >= 0))
		{
            if(!firstPointFound)
			{
                point1 = float3(resX,myY,myZ);
                firstPointFound = true;
            }
			else if(!secondPointFound)
			{
				point2 = float3(resX,myY,myZ);
				secondPointFound = true;
			}
        }


	}
	//check intersection for the plane of Y = 1 if the startpoint is not in this plane
	if((myRay.startpoint.y != resY)&&(myRay.direction.y!=0))
	{
        t = (resY - myRay.startpoint.y) / myRay.direction.y;
        myX = myRay.startpoint.x + myRay.direction.x * t;
        myZ = myRay.startpoint.z + myRay.direction.z * t;
        if ((resX >= myX)&& (myX >= 0)&&(resZ >= myZ)&& (myZ >= 0))
		{
            if(!firstPointFound)
			{
                point1 = float3(myX,resY,myZ);
                firstPointFound = true;
            }
			else if(!secondPointFound)
			{
				point2 = float3(myX,resY,myZ);
				secondPointFound = true;
			}
        }


	}	
	
	//check intersection for the plane of Z = 1 if the startpoint is not in this plane
	if((myRay.startpoint.z != resZ)&&(myRay.direction.z!=0))
	{
        t = (resZ - myRay.startpoint.z) / myRay.direction.z;
        myX = myRay.startpoint.x + myRay.direction.x * t;
        myY = myRay.startpoint.y + myRay.direction.y * t;
        if ((resX >= myX)&& (myX >= 0)&&(resY >= myY)&& (myY >= 0))
		{
            if(!firstPointFound)
			{
                point1 = float3(myX,myY,resZ);
                firstPointFound = true;
            }
			else if(!secondPointFound)
			{
				point2 = float3(myX,myY,resZ);
				secondPointFound = true;
			}
        }


	}

    //check intersection for the plane of X = 0 if the startpoint is not in this plane
	
	if((myRay.startpoint.x != 0) && (myRay.direction.x!=0))
	{
        //get value of t for the point of the line that is on the plane X = -1
		t = (0 - myRay.startpoint.x) / myRay.direction.x;
        //get corresponding Y and Z for the point of the line that is on the plane X = -1
		myY = myRay.startpoint.y + myRay.direction.y * t;
        myZ = myRay.startpoint.z + myRay.direction.z * t;
        //if both Y and Z are between the limits then the point is intersection of line and the cube
		if ((resY >= myY)&& (myY >= 0)&&(resZ >= myZ)&& (myZ >= 0))
		{
            if(!firstPointFound)
			{
                point1 = float3(0, myY, myZ);
                firstPointFound = true;
            }
			else if(!secondPointFound)
			{
				point2 = float3(0, myY, myZ);
				secondPointFound = true;
			}
        }


	}
	
	//check intersection for the plane of Y = -1 if the startpoint is not in this plane
	if((myRay.startpoint.y != 0)&&(myRay.direction.y!=0))
	{
        t = (0 - myRay.startpoint.y) / myRay.direction.y;
        myX = myRay.startpoint.x + myRay.direction.x * t;
        myZ = myRay.startpoint.z + myRay.direction.z * t;
        if ((resX >= myX)&& (myX >= 0)&&(resZ >= myZ)&& (myZ >= 0))
		{
            if(!firstPointFound)
			{
                point1 = float3(myX,0,myZ);
                firstPointFound = true;
            }
			else if(!secondPointFound)
			{
				point2 = float3(myX,0,myZ);
				secondPointFound = true;
			}
        }


	}

	if((myRay.startpoint.z != 0)&&(myRay.direction.z!=0))
	{
        t = (0 - myRay.startpoint.z) / myRay.direction.z;
        myX = myRay.startpoint.x + myRay.direction.x * t;
        myY = myRay.startpoint.y + myRay.direction.y * t;
        if ((resX >= myX)&& (myX >= 0)&&(resY >= myY)&& (myY >= 0))
		{
            if(!firstPointFound)
			{
                point1 = float3(myX,myY,0);
                firstPointFound = true;
            }
			else if(!secondPointFound)
			{
				point2 = float3(myX,myY,0);
				secondPointFound = true;
			}
        }
	}	
	// calculate the position of the ray source (camera position)
	float3 raySource = myRay.secondpoint;
    lightRay entryToExitRay;
    //initialise an array with the entry point is the start and the exit is the secondpoint
	if(distance(raySource, point1) < distance(raySource, point2)) 
	//check which point is closer to the camera position
	{
        entryToExitRay.startpoint = point1;
        entryToExitRay.secondpoint = point2;
    }
	else
	{
        entryToExitRay.startpoint = point2;
        entryToExitRay.secondpoint = point1;
    }


	//get the direction of the ray so the equation of the ray is X = X0 + (X1-X0)t where 0 <= t <=1 between the entry and exit points
	entryToExitRay.direction = entryToExitRay.secondpoint - entryToExitRay.startpoint;
    return entryToExitRay;
}





//searchs for a point on the ray which has texture value >= ISO
float3float searchForISOPoint(lightRay entryToExitRay,PSIn i)
{
    //get the actual distance between the entry and exit points
	float entryExitDistance = distance(entryToExitRay.startpoint,entryToExitRay.secondpoint);
    //the equation of the ray is X = X0 + (X1-X0)t where 0 <= t <=1 between the entry and exit points
	//get the normalized step(delta t) size (0 <= myStep <= 1.0)
	float myStep = 0.001 / entryExitDistance;
    float3 point1 = entryToExitRay.startpoint;
    //the accumolative normalizes distance from entry point
	float tDistanceFromEntry = 0;
    float3 point2 = entryToExitRay.startpoint;
    float result = -1;

    float4 pixelPosition = mul(float4(i.ospos.xyz,1.0),WorldViewProj);
    pixelPosition = pixelPosition/pixelPosition.w;
    float foundDepth = depthTexture.SampleLevel(samLinear,float2((pixelPosition.x + 1)/2,(0 - pixelPosition.y + 1)/2),0);
	
    while(tDistanceFromEntry <= 1)
	{
        float3 testPoint = float3(point1.x / resX, point1.y / resY,point1.z / resZ);

		float pixelW = point1.x * WorldViewProj[0][3] + point1.y * WorldViewProj[1][3] 
				+ point1.z * WorldViewProj[2][3] + WorldViewProj[3][3];
			float pixelZ =  point1.x * WorldViewProj[0][2] + point1.y * WorldViewProj[1][2] 
				+ point1.z * WorldViewProj[2][2] + WorldViewProj[3][2];
			if(pixelZ/pixelW > foundDepth)
			{				
				float3float output;
				output.c = -1;
				output.depth = foundDepth;
				output.normal = 0;
				return output;	
			}
        //check the position
		result = scalarTexture.SampleLevel(samLinear,testPoint,0);
        //if point1 is greater than or equal the ISO value return the texture value at point1		
		if(dispValueMax < result)
		{			
			float3float output;
			//get normal n store it in output.normal
			output.c = result;
			output.depth = pixelZ/pixelW;
			output.normal=float3(0,0,0);
			float TempDist = tDistanceFromEntry;
			float next=0,before=0;
			//get 2 points in X direction
			
			next = scalarTexture.SampleLevel(samLinear,testPoint,0,int3(1,0,0));
			if(testPoint.x < 0.1)
				before = scalarTexture.SampleLevel(samLinear,testPoint,0,int3(0,0,0));
			else
				before = scalarTexture.SampleLevel(samLinear,testPoint,0,int3(-1,0,0));
			output.normal.x=(next-before)/(2);
			//get 2 points in Y direction
			next=scalarTexture.SampleLevel(samLinear,testPoint,0,int3(0,1,0));
			if(testPoint.y < 0.1)
				before=scalarTexture.SampleLevel(samLinear,testPoint,0,int3(0,0,0));
			else
				before=scalarTexture.SampleLevel(samLinear,testPoint,0,int3(0,-1,0));
			output.normal.y=(next-before)/(2);
			//get 2 points in Z direction
			next=scalarTexture.SampleLevel(samLinear,testPoint,0,int3(0,0,1));
			if(testPoint.z < 0.1)
				before=scalarTexture.SampleLevel(samLinear,testPoint,0,int3(0,0,0));
			else
				before=scalarTexture.SampleLevel(samLinear,testPoint,0,int3(0,0,-1));
			output.normal.z=(next-before)/(2);
			
			//output.normal = float3(1,1,1);
			return output;
			
			}


		else
		{
			//move point2 to point1
			point2 = point1;
			//increase the accumolative normalizes distance from entry point
			tDistanceFromEntry = tDistanceFromEntry + myStep;
			//move point1 forward to next point to be tested using the ray equation
			point1 = entryToExitRay.startpoint + tDistanceFromEntry * entryToExitRay.direction;
		}
		
	}
	//if the while loop finished and no value returned then no point on the ray is >= ISO
	float3float output;
    output.c = -1;
	output.depth = 1;
    return output;
}



ParticleDepth blendFragements(lightRay entryToExitRay,PSIn i)
{
    float4 pixelPosition = mul(float4(i.ospos.xyz,1.0),WorldViewProj);
    pixelPosition = pixelPosition/pixelPosition.w;
    float foundDepth = depthTexture.SampleLevel(samLinear,float2((pixelPosition.x + 1)/2,(0 - pixelPosition.y + 1)/2),0);
    float3 CB = {0,0,0};
    float alphaB = 0;
    float3 CF= {0,0,0};
    float alphaF = 0;
    //get the actual distance between the entry and exit points
	float entryExitDistance = distance(entryToExitRay.startpoint,entryToExitRay.secondpoint);
    //the equation of the ray is X = X0 + (X1-X0)t where 0 <= t <=1 between the entry and exit points
	//get the normalized step(delta t) size (0 <= myStep <= 1.0)
	float myStep = 0.005 / entryExitDistance;
    float3 point1 = entryToExitRay.startpoint;
    //the accumolative normalizes distance from entry point
	float tDistanceFromEntry = 0;
    float result = -1;
    float4 curFrag=float4(0,0,0,0);
	ParticleDepth resultParticle;
	resultParticle.color = curFrag;
	resultParticle.depth = 1;
    if(dispValueMin > dispValueMax)
		return resultParticle;
    bool stillBlending = true;
    while((tDistanceFromEntry <= 1)&& ( alphaF < 0.99))
	{
        if(0 <= foundDepth < 1)
		{
            float pixelW = point1.x * WorldViewProj[0][3] + point1.y * WorldViewProj[1][3] 
				+ point1.z * WorldViewProj[2][3] + WorldViewProj[3][3];
			float pixelZ =  point1.x * WorldViewProj[0][2] + point1.y * WorldViewProj[1][2] 
				+ point1.z * WorldViewProj[2][2] + WorldViewProj[3][2];
            if(pixelZ/pixelW >= foundDepth)
			{
				stillBlending = false;
				resultParticle.depth = pixelZ/pixelW;
			}
        }


		//get point1 to the [0,1]3 coordinates
		float3 currentPoint = float3(point1.x / resX,point1.y / resY,point1.z / resZ);
        //check the position
		result = scalarTexture.SampleLevel(samLinear,currentPoint,0);
        if((result < dispValueMin)||(result > dispValueMax))
		{
            tDistanceFromEntry = tDistanceFromEntry + myStep;
            point1 = entryToExitRay.startpoint + tDistanceFromEntry * entryToExitRay.direction;
        }


		else if(stillBlending)
		{
            curFrag = gcolortexture.SampleLevel(samLinear,(result - dispValueMin) /(dispValueMax - dispValueMin),0);
            CB = float3(curFrag.r,curFrag.g,curFrag.b);
            alphaB = curFrag.a * 0.005 / entryExitDistance;
            CF += (1 - alphaF) * CB * alphaB;
            alphaF += (1 - alphaF) * alphaB;
            tDistanceFromEntry = tDistanceFromEntry + myStep;
            point1 = entryToExitRay.startpoint + tDistanceFromEntry * entryToExitRay.direction;
        }


	}
	resultParticle.color = float4(CF,alphaF);
	
    return resultParticle;
}





ParticleDepth VolumePSDVR(PSIn i)
{
    lightRay cameraRay;
    //convert the point-to be calculated- position from [0,1]3 to [-1,1]3
	cameraRay.secondpoint = LightPosition;
    cameraRay.startpoint = i.ospos;
    //calculate direction of the line
	cameraRay.direction = cameraRay.secondpoint - cameraRay.startpoint;
    //get the entry point by calling the function GetIntersection()
	lightRay intersection = GetIntersection(cameraRay);
    return blendFragements(intersection,i);
    
}


//pixel shader that uses intersection 
ParticleDepth VolumePSISO(PSIn i)
{
    lightRay cameraRay;
    //convert the point-to be calculated- position from [0,1]3 to [-1,1]3
	cameraRay.secondpoint = LightPosition;
    cameraRay.startpoint = i.ospos;
    //calculate direction of the line
	cameraRay.direction = cameraRay.startpoint - cameraRay.secondpoint;
    //get the entry point by calling the function GetIntersection()
	lightRay intersection = GetIntersection(cameraRay);
    //return float4(intersection.secondpoint.x/resX,intersection.secondpoint.y/resY,intersection.secondpoint.z/resZ,0.5);
	float3float c2 = searchForISOPoint(intersection,i);
    //we need a struct to return both the value and the normal calculated in the next step
	//convert the entry point position from [-1,1]3 to [0,1]3
	//float4 color= float4((intersection.startpoint.x + 1.0)/2.0,(intersection.startpoint.y + 1.0)/2.0,(intersection.startpoint.z + 1.0)/2.0,1.0);
    if(c2.c != -1)
	{
		ParticleDepth resultParticle;
		resultParticle.depth = c2.depth;
        float4 diff = float4(0.7,0.7,0.7,1);
        resultParticle.color = float4((Lighting(i,-normalize(c2.normal),diff)).xyz,diff.w);
        return resultParticle;
	}
	else
	{
        discard;
        ParticleDepth resultParticle;
		resultParticle.depth = 1;
        resultParticle.color = float4(0,0,0,0);
        return resultParticle;
    }


}


//**************************************************************************************************//


PSIn ProbeVS (uint id : SV_VertexID)  
{
    PSIn result;
    result.nor=0;
    result.lightDir=float3(0,0,0);
    result.ospos=float4(0,0,0,1);
	result.nextVertix = float3(0,0,0);
	result.lightDirNext = float3(0,0,0);
	result.color=float4(0,0,0,0);
    switch(id) {
        default:
		case 0: {
            result.pos= mul(float4( mouseMin.x, mouseMax.y, mouseMin.z, 1.0),WorldViewProj);
            return result;
        }


		case 1:{
            result.pos=mul(float4(mouseMax.x, mouseMax.y, mouseMin.z, 1.0),WorldViewProj) ;
            return result;
        }


		case 2: {
            result.pos=mul(float4( mouseMin.x, mouseMax.y, mouseMin.z, 1.0),WorldViewProj);
            return result;
        }


		case 3:{
            result.pos=mul(float4( mouseMin.x, mouseMin.y, mouseMin.z, 1.0),WorldViewProj) ;
            return result;
        }


		case 4:{
            result.pos=mul(float4(mouseMin.x, mouseMax.y, mouseMin.z, 1.0),WorldViewProj)	;
            return result;
        }


		case 5: {
            result.pos=mul(float4(mouseMin.x, mouseMax.y, mouseMax.z, 1.0),WorldViewProj);
            return result;
        }


		case 6: {
            result.pos=mul(float4(mouseMax.x, mouseMax.y, mouseMin.z, 1.0),WorldViewProj);
            return result;
        }


		case 7: {
            result.pos=mul(float4(mouseMax.x, mouseMax.y, mouseMax.z, 1.0),WorldViewProj);
            return result;
        }


		case 8: {
            result.pos=mul(float4(mouseMax.x, mouseMax.y, mouseMin.z, 1.0),WorldViewProj);
            return result;
        }


		case 9: {
            result.pos=mul(float4(mouseMax.x, mouseMin.y, mouseMin.z, 1.0),WorldViewProj);
            return result;
        }


				//
		case 10: {
            result.pos=mul(float4(mouseMax.x, mouseMax.y, mouseMax.z, 1.0),WorldViewProj);
            return result;
        }


		case 11:{
            result.pos=mul(float4(mouseMax.x, mouseMin.y, mouseMax.z, 1.0),WorldViewProj);
            return result;
        }


		case 12: {
            result.pos=mul(float4(mouseMax.x, mouseMax.y, mouseMax.z, 1.0),WorldViewProj);
            return result;
        }


		case 13:{
            result.pos=mul(float4(  mouseMin.x, mouseMax.y, mouseMax.z, 1.0),WorldViewProj);
            return result;
        }


				//
		case 14: {
            result.pos=mul(float4(mouseMax.x, mouseMin.y, mouseMax.z, 1.0),WorldViewProj);
            return result;
        }


		case 15: {
            result.pos=mul(float4(mouseMax.x, mouseMin.y, mouseMin.z, 1.0),WorldViewProj);
            return result;
        }


				 //
		case 16:{
            result.pos=mul(float4( mouseMin.x, mouseMax.y, mouseMax.z, 1.0),WorldViewProj);
            return result;
        }


		case 17: {
            result.pos=mul(float4( mouseMin.x, mouseMin.y, mouseMax.z, 1.0),WorldViewProj);
            return result;
        }


				 //
		case 18: {
            result.pos=mul(float4( mouseMin.x, mouseMin.y, mouseMax.z, 1.0),WorldViewProj);
            return result;
        }


		case 19:{
            result.pos=mul(float4(mouseMin.x, mouseMin.y, mouseMin.z, 1.0),WorldViewProj);
            return result;
        }


		case 20:{
            result.pos=mul(float4( mouseMin.x, mouseMin.y, mouseMax.z, 1.0),WorldViewProj);
            return result;
        }


		case 21: {
            result.pos=mul(float4( mouseMax.x, mouseMin.y, mouseMax.z, 1.0),WorldViewProj);
            return result;
        }


				 //
		case 22: {
            result.pos=mul(float4( mouseMin.x, mouseMin.y, mouseMin.z, 1.0),WorldViewProj);
            return result;
        }


		case 23: {
            result.pos=mul(float4( mouseMax.x, mouseMin.y, mouseMin.z, 1.0),WorldViewProj);
            return result;
        }


		
    }
}


//pixel shader for solid cube //
float4 ProbePS(PSIn i) : SV_Target
{
    return ProbeColor;
}


//vertex shader for solid cube //
PSIn VolumeVS (uint id : SV_VertexID)  
{
    PSIn result;
    result.nor=0;
    result.lightDir=float3(0,0,0);
	result.nextVertix = float3(0,0,0);
	result.lightDirNext = float3(0,0,0);
	result.color=float4(0,0,0,0);
    switch(id) {
        default:
		case 0: {
            result.pos= mul(float4( 0, resY, 0, 1.0),WorldViewProj);
            result.ospos=float4( 0, resY, 0, 1.0) ;
            return result;
        }


		case 1:{
            result.pos=mul(float4( resX, resY, 0, 1.0),WorldViewProj)	;
            result.ospos=float4( resX, resY, 0, 1.0) ;
            return result;
        }


		case 2: {
            result.pos=mul(float4( 0, 0, 0, 1.0),WorldViewProj);
            result.ospos=float4( 0, 0, 0, 1.0) ;
            return result;
        }


		case 3:{
            result.pos=mul(float4( 0, 0, 0, 1.0),WorldViewProj)	;
            result.ospos=float4( 0, 0, 0, 1.0) ;
            return result;
        }


		case 4:{
            result.pos=mul(float4( resX, resY, 0, 1.0),WorldViewProj)	;
            result.ospos=float4( resX, resY, 0, 1.0) ;
            return result;
        }


		case 5: {
            result.pos=mul(float4( resX, 0, 0, 1.0),WorldViewProj)	;
            result.ospos=float4( resX, 0, 0, 1.0) ;
            return result;
        }


		case 6: {
            result.pos=mul(float4( resX, resY, 0, 1.0),WorldViewProj)	;
            result.ospos=float4( resX, resY, 0, 1.0) ;
            return result;
        }


		case 7: {
            result.pos=mul(float4( resX, resY, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4( resX, resY, resZ, 1.0) ;
            return result;
        }


		case 8: {
            result.pos=mul(float4( resX, 0, 0, 1.0),WorldViewProj)	;
            result.ospos=float4( resX, 0, 0, 1.0) ;
            return result;
        }


		case 9: {
            result.pos=mul(float4( resX, 0, 0, 1.0),WorldViewProj)	;
            result.ospos=float4( resX, 0, 0, 1.0) ;
            return result;
        }


		case 10: {
            result.pos=mul(float4( resX, resY, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4( resX, resY, resZ, 1.0) ;
            return result;
        }


		case 11:{
            result.pos=mul(float4( resX, 0, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4( resX, 0, resZ, 1.0) ;
            return result;
        }


		case 12: {
            result.pos=mul(float4( resX, resY, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4( resX, resY, resZ, 1.0) ;
            return result;
        }


		case 13:{
            result.pos=mul(float4( 0, resY, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4(0, resY, resZ, 1.0) ;
            return result;
        }


		case 14: {
            result.pos=mul(float4( resX, 0, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4( resX, 0, resZ, 1.0) ;
            return result;
        }


		case 15: {
            result.pos=mul(float4( resX, 0, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4( resX, 0, resZ, 1.0) ;
            return result;
        }


		case 16:{
            result.pos=mul(float4( 0, resY, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4(0, resY, resZ, 1.0) ;
            return result;
        }


		case 17: {
            result.pos=mul(float4( 0, 0, resZ, 1.0),WorldViewProj);
            result.ospos=float4(0, 0, resZ, 1.0) ;
            return result;
        }


		case 18: {
            result.pos=mul(float4( 0, resY, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4(0, resY, resZ, 1.0) ;
            return result;
        }


		case 19:{
            result.pos=mul(float4(0, resY, 0, 1.0),WorldViewProj)	;
            result.ospos=float4(0, resY, 0, 1.0) ;
            return result;
        }


		case 20:{
            result.pos=mul(float4( 0, 0, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4(0, 0, resZ, 1.0) ;
            return result;
        }


		case 21: {
            result.pos=mul(float4( 0, 0, resZ, 1.0),WorldViewProj);
            result.ospos=float4(0, 0, resZ, 1.0) ;
            return result;
        }


		case 22: {
            result.pos=mul(float4( 0, resY, 0, 1.0),WorldViewProj);
            result.ospos=float4(0, resY, 0, 1.0) ;
            return result;
        }


		case 23: {
            result.pos=mul(float4( 0, 0, 0, 1.0),WorldViewProj);
            result.ospos=float4(0, 0, 0, 1.0) ;
            return result;
        }


		case 24: {
            result.pos=mul(float4( 0, resY, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4(0, resY, resZ, 1.0) ;
            return result;
        }


		case 25:{
            result.pos=mul(float4( resX, resY, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4( resX, resY, resZ, 1.0) ;
            return result;
        }


		case 26:{
            result.pos=mul(float4(0, resY, 0, 1.0),WorldViewProj)	;
            result.ospos=float4(0, resY, 0, 1.0) ;
            return result;
        }


		case 27:{
            result.pos=mul(float4( 0, resY, 0, 1.0),WorldViewProj)	;
            result.ospos=float4(0, resY, 0, 1.0) ;
            return result;
        }


		case 28: {
            result.pos=mul(float4( resX, resY, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4( resX, resY, resZ, 1.0) ;
            return result;
        }


		case 29: {
            result.pos=mul(float4( resX, resY, 0, 1.0),WorldViewProj)	;
            result.ospos=float4( resX, resY, 0, 1.0) ;
            return result;
        }


		case 30:{
            result.pos=mul(float4(0, 0, 0, 1.0),WorldViewProj)	;
            result.ospos=float4(0, 0, 0, 1.0) ;
            return result;
        }


		case 31: {
            result.pos=mul(float4( resX, 0, 0, 1.0),WorldViewProj)	;
            result.ospos=float4( resX, 0, 0, 1.0) ;
            return result;
        }


		case 32: {
            result.pos=mul(float4( 0, 0, resZ, 1.0),WorldViewProj);
            result.ospos=float4(0, 0, resZ, 1.0) ;
            return result;
        }


		case 33: {
            result.pos=mul(float4( 0, 0, resZ, 1.0),WorldViewProj);
            result.ospos=float4(0, 0, resZ, 1.0) ;
            return result;
        }


		case 34: {
            result.pos=mul(float4( resX, 0, 0, 1.0),WorldViewProj)	;
            result.ospos=float4( resX, 0, 0, 1.0) ;
            return result;
        }


		case 35:{
            result.pos=mul(float4( resX, 0, resZ, 1.0),WorldViewProj)	;
            result.ospos=float4( resX, 0, resZ, 1.0) ;
            return result;
        }


		
    }
}

//pixel shader for solid cube //
float4 VolumePS(PSIn i) : SV_Target
{
    return float4(1,0,0,1);
}



//mesh vertex shader //
PSIn SimpleMeshVS(Vertex v) {
    PSIn result;
	result.pos = mul(float4(v.pos.xyz,1),mul(scalingMatrix,WorldViewProj));
    result.nor = normalize(mul(float4(v.nor,1),World));
    result.ospos=float4(v.pos.xyz,1);
    result.lightDir = normalize(mul(LightPosition,World)-mul(result.ospos.xyz,World));
    return result;
}



//mesh pixel shader 
ParticleDepth SimpleMeshPS(PSIn i) 
	{
	
	float4 pixelPosition = mul(float4(i.ospos.xyz,1.0),WorldViewProj);
    pixelPosition = pixelPosition/pixelPosition.w;
    float foundDepth = depthTexture.SampleLevel(samLinear,float2(i.pos.x,i.pos.y),0);
	if(foundDepth < pixelPosition.z)
	{
		discard;
	}
	ParticleDepth resultParticle;
	resultParticle.depth = pixelPosition.z;
	resultParticle.color = Lighting(i,i.nor,float4(0.5,0.5,0.5,1));
    return resultParticle;
}



//not used anymore as Arrows rendering is disabled in UI
Particle ArrowsVS(uint id: SV_VertexID)
{
    Particle result;
    uint x,y,z;
    uint zQ = id/resZ;
    z = id%resZ;
    uint yQ=zQ/resY;
    y = zQ%resY;
    x = yQ%resX;
    if(pickedZ!=-1)z=pickedZ;
    //result.vec=gtexture.SampleLevel(samLinear,float3((float)x/(float)resX,(float)y/(float)resY,(float)z/(float)resZ),0);
	result.vec=(weight * gtexture.SampleLevel(samLinear,float3((float)x/(float)resX,(float)y/(float)resY,(float)z/(float)resZ),0) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3((float)x/(float)resX,(float)y/(float)resY,(float)z/(float)resZ),0))
					/(weight + weightNext);
    result.pos=mul(float4(x,y,z,1),WorldViewProj);
    result.age=0;
    return result;
}


//geometry shader to create arrows from points (not used anymore)
[maxvertexcount(11)]
void ArrowsGS(point Particle v[1], inout LineStream <Particle> stream)
{
    Particle v2;
    float3 vec1 = v[0].pos;
    float3 vec2 = v[0].vec;
    float3 vec3 = cross(vec2,vec1);
    float3 vec4 = cross(vec3,vec2);
    float3 vec5 = normalize(vec3);
    float3 vec6 = normalize(vec4);
    float3 vec7 = normalize(vec2);
    v2.pos=v[0].pos;
    v2.vec=v[0].vec;
    v2.age=0;
    Particle v3;
    v3.pos = v2.pos;
    v2.worldPos = v2.pos;
    v2.pos=v3.pos + 0.25 * float4(vec5,4);
    stream.Append(v2);
    v2.pos=v3.pos + 0.25 * float4(vec6,4);
    stream.Append(v2);
    v2.pos=v3.pos + float4(vec7,1);
    stream.Append(v2);
    v2.pos=v3.pos + 0.25 * float4(vec5,4);
    stream.Append(v2);
    v2.pos=v3.pos - 0.25 * float4(vec6,-4);
    stream.Append(v2);
    v2.pos=v3.pos + float4(vec7,1);
    stream.Append(v2);
    v2.pos=v3.pos - 0.25 * float4(vec6,-4);
    stream.Append(v2);
    v2.pos=v3.pos - 0.25 * float4(vec5,-4);
    stream.Append(v2);
    v2.pos=v3.pos + float4(vec7,1);
    stream.Append(v2);
    v2.pos=v3.pos - 0.25 * float4(vec5,-4);
    stream.Append(v2);
    v2.pos=v3.pos + 0.25 * float4(vec6,4);
    stream.Append(v2);
}



float4 ArrowsPS(Particle i) : SV_Target 
{
    return float4(0,1,1,1);
}



//particles compute shader
[numthreads(256,1,1)]
void ParticlesCS(uint3 threadID : SV_DispatchThreadID, uint3 groupID : SV_GroupID,uint threadIDInGroup : SV_GroupIndex)
{
    int n=  threadID.z*gridX*gridY + threadID.y*gridX + threadID.x;
    //int n=threadID.x;
	Particle p=gparticlesBuffer[n];
    float4 d=(weight * gtexture.SampleLevel(samLinear,float3(p.pos.x/resX,p.pos.y/resY,p.pos.z/resZ),0) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(p.pos.x/resX,p.pos.y/resY,p.pos.z/resZ),0))
					/(weight + weightNext);
    if(p.pos.x>resX || p.pos.y>resY || p.pos.z>resZ ||p.pos.x<0 || p.pos.y<0 || p.pos.z<0  ||p.age>=maxAge )
	{
        gparticlesBuffer[n]=RandomBuffer[n];
        return;
    }


	p.pos=p.pos+d * stepSize;
    p.age+=stepSize;
    gparticlesBuffer[n]=p;
}


[maxvertexcount(6)]
void ParticlesGS(point Particle v[1], inout TriangleStream <Particle> stream)
{
    Particle v2;
    v2.vec = v[0].pos;
    lightRay cameraRay;
    cameraRay.startpoint = CameraAt;
    cameraRay.secondpoint = v[0].worldPos;
    float cameraSquareDistance;
    float3 squareCenter;
    //calculate direction of the line
	cameraRay.direction = cameraRay.secondpoint - cameraRay.startpoint;
    float cameraParticleDistance = distance(cameraRay.secondpoint,cameraRay.startpoint);
    if(cameraParticleDistance > 0.01)
	{
        cameraSquareDistance = (cameraParticleDistance - 0.01)/cameraParticleDistance;
        squareCenter = cameraRay.startpoint + cameraSquareDistance * cameraRay.direction;
    }


	else
	{
        cameraSquareDistance = 0.01/cameraParticleDistance;
        squareCenter = cameraRay.secondpoint - cameraSquareDistance * cameraRay.direction;
    }


		
	float3 vec1 = squareCenter;
    float3 vec2 = cameraRay.direction;
    float3 vec3 = cross(vec2,vec1);
    float3 vec4 = cross(vec3,vec2);
    float3 vec5 = normalize(vec3);
    float3 vec6 = normalize(vec4);
    float4 center = float4(vec1,1);
    v2.age= v[0].age;
    //draw rectangle around the sphere of radious 0.01 => rectangle vertex to center distance is 0.01/cos(45)
	v2.pos = center + (0.01/cos(45)) * float4(vec5,0);
    v2.worldPos = v2.pos;
    v2.pos=mul(v2.pos,WorldViewProj);
    stream.Append(v2);
    v2.pos = center + (0.01/cos(45)) * float4(vec6,0);
    v2.worldPos = v2.pos;
    v2.pos=mul(v2.pos,WorldViewProj);
    stream.Append(v2);
    v2.pos = center - (0.01/cos(45)) * float4(vec5,0);
    v2.worldPos = v2.pos;
    v2.pos=mul(v2.pos,WorldViewProj);
    stream.Append(v2);
    stream.RestartStrip();
    v2.pos = center + (0.01/cos(45)) * float4(vec5,0);
    v2.worldPos = v2.pos;
    v2.pos=mul(v2.pos,WorldViewProj);
    stream.Append(v2);
    v2.pos = center - (0.01/cos(45)) * float4(vec6,0);
    v2.worldPos = v2.pos;
    v2.pos=mul(v2.pos,WorldViewProj);
    stream.Append(v2);
    v2.pos = center - (0.01/cos(45)) * float4(vec5,0);
    v2.worldPos = v2.pos;
    v2.pos=mul(v2.pos,WorldViewProj);
    stream.Append(v2);
    stream.RestartStrip();
}


Particle ParticlesVS(uint id: SV_VertexID)
{
    Particle p=SpawnBuffer[id];
    p.worldPos = p.pos;
    p.pos=mul(p.pos,WorldViewProj);
    return p;
}


float4 PointsPS(Particle i) : SV_Target 
{
    return particlesColor;
}

ParticleDepth ParticlesPS(Particle i) 
{
    ParticleDepth finalParticle;
    lightRay cameraRay;
    cameraRay.startpoint = CameraAt;
    cameraRay.secondpoint = i.worldPos;
    float cameraSquareDistance;
    float3 squareCenter;
    //calculate direction of the line
	cameraRay.direction = cameraRay.secondpoint - cameraRay.startpoint;
    float3 particlePosition = mul(i.vec,WorldViewProjInverse).xyz;
    //return i.vec;
	float3 intersectionPoint;
    bool intersected = false;
    float squareSphereDistance = 0;
    float cameraPixleDistance = distance(cameraRay.secondpoint,cameraRay.startpoint);
    while(!intersected && squareSphereDistance < 0.02)
	{
        intersectionPoint = cameraRay.startpoint + (1 + (squareSphereDistance /cameraPixleDistance)) * cameraRay.direction;
        float3 distanceT = intersectionPoint - particlePosition;
        if(distance(particlePosition,intersectionPoint) <= 0.01)
		{
            intersected = true;
        }


		squareSphereDistance += 0.001;
    }

	
	if(!intersected)
	{
        discard;
        finalParticle.depth = 1.0;
        finalParticle.color = particlesColor;
        return finalParticle;
    }


	else
	{
        float pixelW = intersectionPoint.x * WorldViewProj[0][3] + intersectionPoint.y * WorldViewProj[1][3] 
		+ intersectionPoint.z * WorldViewProj[2][3] + WorldViewProj[3][3];
        float pixelZ =  intersectionPoint.x * WorldViewProj[0][2] + intersectionPoint.y * WorldViewProj[1][2] 
		+ intersectionPoint.z * WorldViewProj[2][2] + WorldViewProj[3][2];
        finalParticle.depth = pixelZ/pixelW;
        float3 lightCurrentPos = LightPosition;
        float3 lightDirec = mul(LightPosition,World)-mul(particlePosition,World);
        float4 colorOutput = {
            0,0,0,1};
        float4 ambient = {
            0.1,0.1,0.1,1};
        float4 diffuse = particlesColor;
        float d = i.pos.z/i.pos.w;
		float ambientIntesity = 1;
        float diffuseIntensity = 4;
		//float3 l=normalize(mul(LightPosition,World)-mul(i.worldPos,World));
		float3 l=LightDirection;
        float3 Normal = normalize(mul(intersectionPoint-particlePosition,World));
	    //float3 Normal=normalize(mul(cross(intersectionPoint,particlePosition),World)); //must get this right !!!!
        float NdotL	=	saturate(dot( Normal , l));
		float3 Reflect = normalize(2 *Normal * NdotL + l);
        float4 specular = pow(saturate(dot(Reflect, l)),100);
        colorOutput+=ambient*ambientIntesity+(diffuse*NdotL*diffuseIntensity);//+specular*0.01;
        finalParticle.color = colorOutput;
        return finalParticle;
       
	}
}

//velocity magnitude compute shader
[numthreads(10,10,10)] //1000 threads per block
void VelocityMagCS(uint3 threadID : SV_DispatchThreadID, uint3 groupID : SV_GroupID,uint threadIDInGroup : SV_GroupIndex)
{
    //for the few extra threads
	if(threadID.x > resX|| threadID.y > resY || threadID.z > resZ ) 
	return;
    float4 input=(weight * gtexture.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),0) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),0))
					/(weight + weightNext);

	RWscalarTexture[threadID]=length(input)*flipValue;
}



//divergence compute shader
[numthreads(10,10,10)] //1000 threads per block
void DivergenceCS(uint3 threadID : SV_DispatchThreadID, uint3 groupID : SV_GroupID,uint threadIDInGroup : SV_GroupIndex)
{
    //for the few extra threads
	if(threadID.x > resX || threadID.y > resY || threadID.z > resZ ) 
	return;
    float4 input=(weight * gtexture.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),0) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),0))
					/(weight + weightNext);
    float4 Positive=(weight * gtexture.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(1,0,0)) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(1,0,0)))
					/(weight + weightNext);
    float4 Negative=(weight * gtexture.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(-1,0,0)) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(-1,0,0)))
					/(weight + weightNext);
    float output=(Positive.x-Negative.x)/(sliceThicknessX*2);
    //X derivative

	Positive=(weight * gtexture.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,1,0)) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,1,0)))
					/(weight + weightNext);
    Negative=(weight * gtexture.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,-1,0)) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,-1,0)))
					/(weight + weightNext);
    output+=(Positive.y-Negative.y)/(sliceThicknessY*2);
    //add Y derivative

	Positive=(weight * gtexture.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,0,1)) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,0,1)))
					/(weight + weightNext);
    Negative=(weight * gtexture.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,0,-1)) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,0,-1)))
					/(weight + weightNext);
    output+=(Positive.z-Negative.z)/(sliceThicknessZ*2);
    //add Z derivative
	
	RWscalarTexture[threadID]=output*flipValue;
}


//Vorticity Magnitude CS
[numthreads(10,10,10)] //1000 threads per block
void VorticityMagCS(uint3 threadID : SV_DispatchThreadID, uint3 groupID : SV_GroupID,uint threadIDInGroup : SV_GroupIndex)
{
    //for the few extra threads
	if(threadID.x > resX || threadID.y > resY || threadID.z > resZ ) 
	return;
    float4 input=(weight * gtexture.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),0) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),0))
					/(weight + weightNext);
    float4 PositiveX=(weight * gtexture.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(1,0,0)) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(1,0,0)))
					/(weight + weightNext);
    float4 NegativeX=(weight * gtexture.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(-1,0,0)) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(-1,0,0)))
					/(weight + weightNext);
    float4 PositiveY=(weight * gtexture.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,1,0)) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,1,0)))
					/(weight + weightNext);
    float4 NegativeY=(weight * gtexture.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,-1,0)) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,-1,0)))
					/(weight + weightNext);
    float4 PositiveZ=(weight * gtexture.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,0,1)) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,0,1)))
					/(weight + weightNext);
    float4 NegativeZ=(weight * gtexture.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,0,-1)) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,0,-1)))
					/(weight + weightNext);
    float uzy=(PositiveY.z-NegativeY.z)/(sliceThicknessY*2);
    float uyz=(PositiveZ.y-NegativeZ.y/(sliceThicknessZ*2));
    float uxz=(PositiveZ.x-NegativeZ.x)/(sliceThicknessZ*2);
    float uzx=(PositiveX.z-NegativeX.z)/(sliceThicknessX*2);
    float uyx=(PositiveX.y-NegativeX.y)/(sliceThicknessX*2);
    float uxy=(PositiveY.x-NegativeY.x)/(sliceThicknessY*2);
    float3 curl;
    curl.x=uzy-uyz;
    curl.y=uxz-uzx;
    curl.z=uyx-uxy;
    RWscalarTexture[threadID]=length(curl);
	RWscalarTexture[threadID]=length(curl)*flipValue;
}

[numthreads(10,10,10)] //1000 threads per block
void QCS(uint3 threadID : SV_DispatchThreadID, uint3 groupID : SV_GroupID,uint threadIDInGroup : SV_GroupIndex)
{
    //for the few extra threads
	if(threadID.x > resX || threadID.y > resY || threadID.z > resZ ) 
	return;
    float4 input=(weight * gtexture.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),0) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),0))
					/(weight + weightNext);
    float4 PositiveX=(weight * gtexture.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(1,0,0)) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(1,0,0)))
					/(weight + weightNext);
    float4 NegativeX=(weight * gtexture.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(-1,0,0)) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(-1,0,0)))
					/(weight + weightNext);
    float4 PositiveY=(weight * gtexture.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,1,0)) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,1,0)))
					/(weight + weightNext);
    float4 NegativeY=(weight * gtexture.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,-1,0)) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,-1,0)))
					/(weight + weightNext);
    float4 PositiveZ=(weight * gtexture.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,0,1)) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,0,1)))
					/(weight + weightNext);
    float4 NegativeZ=(weight * gtexture.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,0,-1)) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(threadID.x/resX,threadID.y/resY,threadID.z/resZ),int3(0,0,-1)))
					/(weight + weightNext);
    //okay lets organize this, first i should compute Qs, for that i need S, so let's create the gradiant tensor matrix
	float uzz=(PositiveZ.z-NegativeZ.z)/(sliceThicknessZ*2);
    float uzy=(PositiveY.z-NegativeY.z)/(sliceThicknessY*2);
    float uyz=(PositiveZ.y-NegativeZ.y/(sliceThicknessZ*2));
    float uxx=(PositiveX.x-NegativeX.x)/(sliceThicknessX*2);
    float uxz=(PositiveZ.x-NegativeZ.x)/(sliceThicknessZ*2);
    float uzx=(PositiveX.z-NegativeX.z)/(sliceThicknessX*2);
    float uyy=(PositiveY.y-NegativeY.y)/(sliceThicknessY*2);
    float uyx=(PositiveX.y-NegativeX.y)/(sliceThicknessX*2);
    float uxy=(PositiveY.x-NegativeY.x)/(sliceThicknessY*2);
    matrix<float,3,3> J=
	{
        uxx,uxy,uxz,
		uyx,uyy,uyz,
		uzx,uzy,uzz
	};
    matrix<float,3,3> Jt=transpose(J);
    matrix<float,3,3> S=0.5*(J+Jt);
    matrix<float,3,3> S2=mul(S,S);
    float traceS2= S2[0][0]+S[1][1]+S[2][2];
    float Qs=-0.5*traceS2;
    //okay now to get Qomega= 1/4*||curl||_squared
	float3 curl;
    curl.x=uzy-uyz;
    curl.y=uxz-uzx;
    curl.z=uyx-uxy;
    float Qo=0.25*length(curl)*length(curl);
    float Q=Qo+Qs;
    RWscalarTexture[threadID]=Q;

	RWscalarTexture[threadID]=Q*flipValue;
	}

PSIn StreamLinesVS(uint id: SV_VertexID)
{
	Vertex v=glinesReadBuffer[lengthOfLine*iteration+id];
    PSIn p;
	p.ospos = v.pos;
	p.pos = v.pos;
	p.nor = v.nor;
	if(id < lengthOfLine-1)
	{
		p.nextVertix = glinesReadBuffer[lengthOfLine*iteration+id + 1].pos;
		float4 input=(weight * gtexture.SampleLevel(samLinear,float3(p.nextVertix.x/resX,p.nextVertix.y/resY,p.nextVertix.z/resZ),0) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(p.nextVertix.x/resX,p.nextVertix.y/resY,p.nextVertix.z/resZ),0))
					/(weight + weightNext);
		p.lightDirNext = input.xyz;
	}
	else
	{
		p.nextVertix = p.ospos;
		
	}
	float4 input=(weight * gtexture.SampleLevel(samLinear,float3(v.pos.x/resX,v.pos.y/resY,v.pos.z/resZ),0) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(v.pos.x/resX,v.pos.y/resY,v.pos.z/resZ),0))
					/(weight + weightNext);
	p.lightDir=input.xyz;
    return p;
	
}

[maxvertexcount(30)]
void StreamLinesGS(point PSIn v[1], inout TriangleStream <CylinderSpherePSIn> stream)
{
    CylinderSpherePSIn v2;
    v2.vec = v[0].pos;
	v2.lightDir = v[0].lightDir;
    lightRay cameraRay;
    cameraRay.startpoint = CameraAt;
    cameraRay.secondpoint = v[0].ospos;
    float cameraSquareDistance,cameraSquareDistance2;
    float3 squareCenter,squareCenter2;
    //calculate direction of the line
	cameraRay.direction = cameraRay.secondpoint - cameraRay.startpoint;
    float cameraParticleDistance = distance(cameraRay.secondpoint,cameraRay.startpoint);
    if(cameraParticleDistance > 0.01)
	{
        cameraSquareDistance = (cameraParticleDistance - 0.01)/cameraParticleDistance;
		cameraSquareDistance2 = (cameraParticleDistance + 0.01)/cameraParticleDistance;
        squareCenter = cameraRay.startpoint + cameraSquareDistance * cameraRay.direction;
		squareCenter2 = cameraRay.startpoint + cameraSquareDistance2 * cameraRay.direction;
    }


	else
	{
        cameraSquareDistance = 0.01/cameraParticleDistance;
		cameraSquareDistance2 = -0.01/cameraParticleDistance;
        squareCenter = cameraRay.secondpoint - cameraSquareDistance * cameraRay.direction;
		squareCenter2 = cameraRay.secondpoint - cameraSquareDistance2 * cameraRay.direction;
    }
	if(distance(v[0].ospos,v[0].nextVertix) > 0.0001)
	{
		v2.vec = v[0].pos;
		v2.type = 1;
		lightRay cameraSecondRay;
		cameraSecondRay.startpoint = CameraAt;
		cameraSecondRay.secondpoint = v[0].nextVertix;
		float cameraSecondSquareDistance,cameraSecondSquareDistance2;
		float3 secondSquareCenter,secondSquareCenter2;
		//calculate direction of the line
		cameraSecondRay.direction = cameraSecondRay.secondpoint - cameraSecondRay.startpoint;
		float cameraSecondParticleDistance = distance(cameraSecondRay.secondpoint,cameraSecondRay.startpoint);
		if(cameraSecondParticleDistance > 0.01)
		{
			cameraSecondSquareDistance = (cameraSecondParticleDistance - 0.01)/cameraSecondParticleDistance;
			cameraSecondSquareDistance2 = (cameraSecondParticleDistance + 0.01)/cameraSecondParticleDistance;
			secondSquareCenter = cameraSecondRay.startpoint + cameraSecondSquareDistance * cameraSecondRay.direction;
			secondSquareCenter2 = cameraSecondRay.startpoint + cameraSecondSquareDistance2 * cameraSecondRay.direction;
		}
		else
		{
			cameraSecondSquareDistance = 0.01/cameraSecondParticleDistance;
			secondSquareCenter = cameraSecondRay.secondpoint - cameraSecondSquareDistance * cameraSecondRay.direction;
			secondSquareCenter2 = cameraSecondRay.secondpoint + cameraSecondSquareDistance * cameraSecondRay.direction;
		}
		v2.axis.startpoint = v[0].ospos;
		v2.axis.secondpoint = v[0].nextVertix;
		v2.axis.direction =  v[0].nextVertix - v[0].ospos;
		v2.origin = float3(0,0,0);
		float3 vec11 = secondSquareCenter - squareCenter;
		float3 vec12 = cameraRay.direction;
		float3 vec21 = secondSquareCenter - squareCenter;
		float3 vec22 = cameraSecondRay.direction;
		float3 vec13 = cross(vec12,vec11);
		float3 vec23 = cross(vec22,vec21);
		float3 vec51 = normalize(vec13);
		float3 vec61 = normalize(vec23);

		float3 vec112 = secondSquareCenter2 - squareCenter2;
		float3 vec122 = cameraRay.direction;
		float3 vec212 = secondSquareCenter2 - squareCenter2;
		float3 vec222 = cameraSecondRay.direction;
		float3 vec132 = cross(vec122,vec112);
		float3 vec232 = cross(vec222,vec212);
		float3 vec52 = normalize(vec132);
		float3 vec62 = normalize(vec232);
		
		//draw rectangle around the sphere of radious 0.01 => rectangle vertex to center distance is 0.01/cos(45)
		v2.pos = float4(squareCenter,1) + 0.01 * float4(vec51,0);
		v2.worldPos = v2.pos;
		v2.lightDir = v[0].lightDir;
		v2.pos=mul(v2.pos,WorldViewProj);
		stream.Append(v2);
		v2.pos = float4(squareCenter,1) - 0.01 * float4(vec51,0);
		v2.worldPos = v2.pos;
		v2.pos=mul(v2.pos,WorldViewProj);
		stream.Append(v2);
		v2.pos = float4(secondSquareCenter,1) + 0.01 * float4(vec61,0);
		v2.worldPos = v2.pos;
		v2.pos=mul(v2.pos,WorldViewProj);
		v2.lightDir = v[0].lightDirNext;
		stream.Append(v2);
		stream.RestartStrip();
		v2.pos = float4(secondSquareCenter,1) + 0.01 * float4(vec61,0);
		v2.worldPos = v2.pos;
		v2.pos=mul(v2.pos,WorldViewProj);
		stream.Append(v2);
		v2.lightDir = v[0].lightDir;
		v2.pos = float4(squareCenter,1) - 0.01 * float4(vec51,0);
		v2.worldPos = v2.pos;
		v2.pos=mul(v2.pos,WorldViewProj);
		stream.Append(v2);
		v2.lightDir = v[0].lightDirNext;
		v2.pos = float4(secondSquareCenter,1) - 0.01 * float4(vec61,0);
		v2.worldPos = v2.pos;
		v2.pos=mul(v2.pos,WorldViewProj);
		stream.Append(v2);
		stream.RestartStrip();

		v2.pos = float4(squareCenter2,1) + 0.01 * float4(vec52,0);
		v2.worldPos = v2.pos;
		v2.lightDir = v[0].lightDir;
		v2.pos=mul(v2.pos,WorldViewProj);
		stream.Append(v2);
		v2.pos = float4(squareCenter2,1) - 0.01 * float4(vec52,0);
		v2.worldPos = v2.pos;
		v2.pos=mul(v2.pos,WorldViewProj);
		stream.Append(v2);
		v2.pos = float4(secondSquareCenter2,1) + 0.01 * float4(vec62,0);
		v2.worldPos = v2.pos;
		v2.pos=mul(v2.pos,WorldViewProj);
		v2.lightDir = v[0].lightDirNext;
		stream.Append(v2);
		stream.RestartStrip();
		v2.pos = float4(secondSquareCenter2,1) + 0.01 * float4(vec62,0);
		v2.worldPos = v2.pos;
		v2.pos=mul(v2.pos,WorldViewProj);
		stream.Append(v2);
		v2.lightDir = v[0].lightDir;
		v2.pos = float4(squareCenter2,1) - 0.01 * float4(vec52,0);
		v2.worldPos = v2.pos;
		v2.pos=mul(v2.pos,WorldViewProj);
		stream.Append(v2);
		v2.lightDir = v[0].lightDirNext;
		v2.pos = float4(secondSquareCenter2,1) - 0.01 * float4(vec62,0);
		v2.worldPos = v2.pos;
		v2.pos=mul(v2.pos,WorldViewProj);
		stream.Append(v2);
		stream.RestartStrip();

		v2.pos = float4(squareCenter2,1) + 0.01 * float4(vec52,0);
		v2.worldPos = v2.pos;
		v2.lightDir = v[0].lightDir;
		v2.pos=mul(v2.pos,WorldViewProj);
		stream.Append(v2);
		v2.pos = float4(squareCenter,1) + 0.01 * float4(vec51,0);
		v2.worldPos = v2.pos;
		v2.pos=mul(v2.pos,WorldViewProj);
		stream.Append(v2);
		v2.pos = float4(secondSquareCenter2,1) + 0.01 * float4(vec62,0);
		v2.worldPos = v2.pos;
		v2.pos=mul(v2.pos,WorldViewProj);
		v2.lightDir = v[0].lightDirNext;
		stream.Append(v2);
		stream.RestartStrip();
		v2.pos = float4(squareCenter2,1) + 0.01 * float4(vec52,0);
		v2.worldPos = v2.pos;
		v2.lightDir = v[0].lightDir;
		v2.pos=mul(v2.pos,WorldViewProj);
		stream.Append(v2);
		v2.pos = float4(squareCenter,1) + 0.01 * float4(vec51,0);
		v2.worldPos = v2.pos;
		v2.pos=mul(v2.pos,WorldViewProj);
		stream.Append(v2);
		v2.pos = float4(secondSquareCenter,1) + 0.01 * float4(vec61,0);
		v2.worldPos = v2.pos;
		v2.pos=mul(v2.pos,WorldViewProj);
		v2.lightDir = v[0].lightDirNext;
		stream.Append(v2);
		stream.RestartStrip();

		v2.pos = float4(squareCenter2,1) - 0.01 * float4(vec52,0);
		v2.worldPos = v2.pos;
		v2.lightDir = v[0].lightDir;
		v2.pos=mul(v2.pos,WorldViewProj);
		stream.Append(v2);
		v2.pos = float4(squareCenter,1) - 0.01 * float4(vec51,0);
		v2.worldPos = v2.pos;
		v2.pos=mul(v2.pos,WorldViewProj);
		stream.Append(v2);
		v2.pos = float4(secondSquareCenter2,1) - 0.01 * float4(vec62,0);
		v2.worldPos = v2.pos;
		v2.pos=mul(v2.pos,WorldViewProj);
		v2.lightDir = v[0].lightDirNext;
		stream.Append(v2);
		stream.RestartStrip();
		v2.pos = float4(squareCenter2,1) - 0.01 * float4(vec52,0);
		v2.worldPos = v2.pos;
		v2.lightDir = v[0].lightDir;
		v2.pos=mul(v2.pos,WorldViewProj);
		stream.Append(v2);
		v2.pos = float4(squareCenter,1) - 0.01 * float4(vec51,0);
		v2.worldPos = v2.pos;
		v2.pos=mul(v2.pos,WorldViewProj);
		stream.Append(v2);
		v2.pos = float4(secondSquareCenter,1) - 0.01 * float4(vec61,0);
		v2.worldPos = v2.pos;
		v2.pos=mul(v2.pos,WorldViewProj);
		v2.lightDir = v[0].lightDirNext;
		stream.Append(v2);
		stream.RestartStrip();
	}
	float3 vec1 = squareCenter;
    float3 vec2 = cameraRay.direction;
    float3 vec3 = cross(vec2,vec1);
    float3 vec4 = cross(vec3,vec2);
    float3 vec5 = normalize(vec3);
    float3 vec6 = normalize(vec4);
    float4 center = float4(vec1,1);
	v2.lightDir = v[0].lightDir;
    //draw rectangle around the sphere of radious 0.01 => rectangle vertex to center distance is 0.01/cos(45)
	v2.type = 0;
	v2.pos = center + (0.01/cos(45)) * float4(vec5,0);
	v2.origin = v[0].ospos;
    v2.worldPos = v2.pos;
    v2.pos=mul(v2.pos,WorldViewProj);
    stream.Append(v2);
    v2.pos = center + (0.01/cos(45)) * float4(vec6,0);
    v2.worldPos = v2.pos;
    v2.pos=mul(v2.pos,WorldViewProj);
    stream.Append(v2);
    v2.pos = center - (0.01/cos(45)) * float4(vec5,0);
    v2.worldPos = v2.pos;
    v2.pos=mul(v2.pos,WorldViewProj);
    stream.Append(v2);
    stream.RestartStrip();
    v2.pos = center + (0.01/cos(45)) * float4(vec5,0);
    v2.worldPos = v2.pos;
    v2.pos=mul(v2.pos,WorldViewProj);
    stream.Append(v2);
    v2.pos = center - (0.01/cos(45)) * float4(vec6,0);
    v2.worldPos = v2.pos;
    v2.pos=mul(v2.pos,WorldViewProj);
    stream.Append(v2);
    v2.pos = center - (0.01/cos(45)) * float4(vec5,0);
    v2.worldPos = v2.pos;
    v2.pos=mul(v2.pos,WorldViewProj);
    stream.Append(v2);
    stream.RestartStrip();
}
ParticleDepth StreamLinesPS(CylinderSpherePSIn i)
{
	ParticleDepth finalParticle;
	if(i.type == 0)
	{		
		lightRay cameraRay;
		cameraRay.startpoint = LightPosition;
		cameraRay.secondpoint = i.worldPos;
		float cameraSquareDistance;
		float3 squareCenter;
		//calculate direction of the line
		cameraRay.direction = cameraRay.secondpoint - cameraRay.startpoint;
		float3 particlePosition = i.origin.xyz;
		float3 intersectionPoint;
		bool intersected = false;
		float squareSphereDistance = 0;
		float cameraPixleDistance = distance(cameraRay.secondpoint,cameraRay.startpoint);
		while(!intersected && squareSphereDistance < 0.02)
		{
			intersectionPoint = cameraRay.startpoint + (1 + (squareSphereDistance /cameraPixleDistance)) * cameraRay.direction;
			float3 distanceT = intersectionPoint - particlePosition;
			if(distance(particlePosition,intersectionPoint) <= (0.0025 + 0.005 * (length(i.lightDir.xyz)/1.7321) ))
			{
				intersected = true;
			}
			squareSphereDistance += 0.001;
		}

	
		if(!intersected)
		{
			discard;
			finalParticle.depth = 0.0;
			finalParticle.color = float4(i.lightDir.xyz,1);
			return finalParticle;
		}


		else
		{
			float4 intersectionWorld = mul(float4(intersectionPoint.xyz,1.0),WorldViewProj);
			intersectionWorld = intersectionWorld/intersectionWorld.w;
			finalParticle.depth = intersectionWorld.z;
			PSIn p;
			p.ospos = float4(intersectionPoint.xyz,1.0);
			
			finalParticle.color = Lighting(p,normalize(mul((intersectionPoint - particlePosition),World)),float4(0,0,1,1));
			//discard;
			return finalParticle;
			
		}
	}
	else if(i.type == 1)
	{		
		lightRay cameraRay;
		cameraRay.startpoint = LightPosition;
		cameraRay.secondpoint = i.worldPos;
		//calculate direction of the line
		cameraRay.direction = cameraRay.secondpoint - cameraRay.startpoint;

		float3 intersectionPoint;
		bool intersected = false;
		float squareSphereDistance = 0;
		float cameraPixleDistance = distance(cameraRay.secondpoint,cameraRay.startpoint);
		float3 circleCenter;
		while(!intersected && squareSphereDistance < 0.01)
		{
			intersectionPoint = cameraRay.startpoint + (1 + (squareSphereDistance /cameraPixleDistance)) * cameraRay.direction;
			float3 line1 = i.axis.startpoint - intersectionPoint;
			float a = length(cross(line1,i.axis.direction));
			float b = length(i.axis.direction);
			if(a/b <= (0.0025 + 0.005 * (length(i.lightDir.xyz)/1.7321) ))
			{
				intersected = true;
			}
			squareSphereDistance += 0.001;
		}

	
		if(!intersected)
		{
			discard;
			finalParticle.depth = 0.0;
			finalParticle.color = float4(0,0,1,1);
			return finalParticle;
		}
		else
		{
			float4 intersectionWorld = mul(float4(intersectionPoint.xyz,1.0),WorldViewProj);
			intersectionWorld = intersectionWorld/intersectionWorld.w;
			finalParticle.depth = intersectionWorld.z;
			PSIn p;
			p.ospos = float4(intersectionPoint.xyz,1.0);
			float a = dot(i.axis.secondpoint-intersectionPoint,i.axis.secondpoint - i.axis.startpoint);
			float b = (-1) * dot(i.axis.startpoint-intersectionPoint,i.axis.secondpoint - i.axis.startpoint);
			finalParticle.color = Lighting(p,normalize(mul(intersectionPoint -(a * i.axis.startpoint + b *  i.axis.secondpoint),World)),float4(0,0,1,1));
			//discard;
			return finalParticle;
			
		}
	}
	finalParticle.depth = 0.0;
	discard;
    return finalParticle;
}

[numthreads(256,1,1)] //256 threads per block
void StreamLinesCS(uint3 threadID : SV_DispatchThreadID, uint3 groupID : SV_GroupID,uint threadIDInGroup : SV_GroupIndex)
{
	 int n= threadID.x;
	 
	 if(n>numberOfLines ) 
		return;

    for(int i=0;i<lengthOfLine-1;i++)
	{
	Vertex v=glinesBuffer[n*lengthOfLine+i];
    float4 d=(weight * gtexture.SampleLevel(samLinear,float3(v.pos.x/resX,v.pos.y/resY,v.pos.z/resZ),0) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(v.pos.x/resX,v.pos.y/resY,v.pos.z/resZ),0))
					/(weight + weightNext);

	v.pos=v.pos+d * integrationStepSize;
    glinesBuffer[n*lengthOfLine+i+1]=v;
	}

}


PSIn RibbonsVS(uint id: SV_VertexID)
{
	Vertex v=gribbonsReadBuffer[lengthOfLine*iteration+id];
	Vertex v2=gribbonsReadBuffer[lengthOfLine*iteration+id+1];
    PSIn p;
	p.ospos = v.pos;
	p.pos=mul(v.pos,WorldViewProj);
	p.nor=normalize(LightPosition-v.pos); //this seems to work better
	//p.nor=normalize(cross(v.pos,v2.pos));
	float4 input=(weight * gtexture.SampleLevel(samLinear,float3(v.pos.x/resX,v.pos.y/resY,v.pos.z/resZ),0) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(v.pos.x/resX,v.pos.y/resY,v.pos.z/resZ),0))
					/(weight + weightNext);
	p.lightDir=input.xyz;
    return p;
	
}

float4 RibbonsPS(PSIn i) : SV_Target 
{
	float3 color=i.lightDir;
	return Lighting(i,i.nor,float4(color,1));
  //  return float4(i.lightDir.xyz,1);
}

[numthreads(256,1,1)] //256 threads per block
void RibbonsCS(uint3 threadID : SV_DispatchThreadID, uint3 groupID : SV_GroupID,uint threadIDInGroup : SV_GroupIndex)
{
	 int n= threadID.x;
	 
	 if(n>numberOfLines ) 
		return;

    for(int i=0;i<lengthOfLine-3;i+=2)
	{
	//float3 normalLine = cross(gribbonsBuffer[n*lengthOfLine+i].pos.xyz,gribbonsBuffer[n*lengthOfLine+i+1].pos.xyz);
	Vertex v=gribbonsBuffer[n*lengthOfLine+i];
    float4 d=(weight * gtexture.SampleLevel(samLinear,float3(v.pos.x/resX,v.pos.y/resY,v.pos.z/resZ),0) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(v.pos.x/resX,v.pos.y/resY,v.pos.z/resZ),0))
					/(weight + weightNext);

	v.pos=v.pos+d * integrationStepSize;
	//v.nor = normalLine;
    gribbonsBuffer[n*lengthOfLine+i+2]=v;
	//2nd vertex
	v=gribbonsBuffer[n*lengthOfLine+i+1];
	d=(weight * gtexture.SampleLevel(samLinear,float3(v.pos.x/resX,v.pos.y/resY,v.pos.z/resZ),0) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(v.pos.x/resX,v.pos.y/resY,v.pos.z/resZ),0))
					/(weight + weightNext);
	v.pos=v.pos+d * integrationStepSize;
	//v.nor = normalLine;
    gribbonsBuffer[n*lengthOfLine+i+3]=v;
	
	}

}

PSIn StreakLinesVS(uint id: SV_VertexID)
{
	float s=gsepReadBuffer[iteration];
	//the seperator also represents how many particles are on the left of the seperator
	int x=streakLinesLength-s; //x is how many particles are on the right of the seperator
	Particle v,v2;
	int sub,sub2=0;
	sub=streakLinesLength*iteration+id;
	v=gstreakReadBuffer[sub];
	if(id==streakLinesLength-1) 
	{
		sub2=streakLinesLength*iteration;
		v2=gstreakReadBuffer[sub2];
	}
	else 
	{
		sub2=streakLinesLength*iteration+id+1;
		v2=gstreakReadBuffer[sub2];
	}

	PSIn p;
	
	p.ospos = v.pos;
	p.pos=mul(v.pos,WorldViewProj);
	p.lightDirNext.x=v.age;
	p.nextVertix=v2.pos.xyz;
	p.lightDirNext.y=v2.age;
	p.nor.x=sub;
	p.nor.y=sub2;
	p.nor.z=v.vec.z;
    return p;
	
}

[maxvertexcount(6)]
void StreakLinesGS(point PSIn v[1], inout TriangleStream <PSIn> stream)
{
PSIn v2;
Particle p1,p2,p3,p4;
p1=gstreakReadBuffer[v[0].nor.x];
p2=gstreakReadBuffer[v[0].nor.y];
p3=gstreakReadBuffer[v[0].nor.x+streakLinesLength];
p4=gstreakReadBuffer[v[0].nor.y+streakLinesLength];
float3 color=float3(0,1,0);
v2.lightDir = v2.nor = v2.nextVertix = v2.lightDirNext = float3(0,0,0);
v2.ospos = p1.pos;
v2.nor.z=p1.vec.z;
v2.pos=mul(v2.ospos,WorldViewProj);
v2.color=float4(color.xyz,v2.nor.z);
stream.Append(v2);
v2.ospos =p4.pos;
v2.nor.z=p4.vec.z;
v2.pos=mul(v2.ospos,WorldViewProj);
v2.color=float4(color.xyz,v2.nor.z);
stream.Append(v2);
v2.ospos =p2.pos;
v2.nor.z=p2.vec.z;
v2.pos=mul(v2.ospos,WorldViewProj);
v2.color=float4(color.xyz,v2.nor.z);
stream.Append(v2);
stream.RestartStrip();

v2.ospos = p1.pos;
v2.nor.z=p1.vec.z;
v2.pos=mul(v2.ospos,WorldViewProj);
v2.color=float4(color.xyz,v2.nor.z);
stream.Append(v2);
v2.ospos =p4.pos;
v2.nor.z=p4.vec.z;
v2.pos=mul(v2.ospos,WorldViewProj);
v2.color=float4(color.xyz,v2.nor.z);
stream.Append(v2);
v2.ospos =p3.pos;
v2.nor.z=p3.vec.z;
v2.pos=mul(v2.ospos,WorldViewProj);
v2.color=float4(color.xyz,v2.nor.z);
stream.Append(v2);
stream.RestartStrip();
}





float4 StreakLinesPS(PSIn i) : SV_Target 
{

   return i.color;
}

[numthreads(512,1,1)] //512 threads per block
void StreakLinesCS(uint3 threadID : SV_DispatchThreadID, uint3 groupID : SV_GroupID,uint threadIDInGroup : SV_GroupIndex)
{
	int n= threadID.x; 
	if(n>numberOfLines*streakLinesLength ) return;
	uint lineIndex=n/streakLinesLength;
	float ageDiff,newAge;
	Particle p=gstreakBuffer[n];
    float4 d=(weight * gtexture.SampleLevel(samLinear,float3(p.pos.x/resX,p.pos.y/resY,p.pos.z/resZ),0) 
					+ weightNext * gtextureNext.SampleLevel(samLinear,float3(p.pos.x/resX,p.pos.y/resY,p.pos.z/resZ),0))
					/(weight + weightNext);
	newAge=p.age+timeSinceLastFrame;
	if(p.age>=lifetime || newAge>=lifetime)
	{
	float delta=p.age-lifetime;
	p.age=-seedingInterval+delta;
	p.pos=gspawnReadBuffer[lineIndex].pos;
	gstreakBuffer[n]=p;
	return;
	}
	 if(p.pos.x>resX || p.pos.y>resY || p.pos.z>resZ ||p.pos.x<0 || p.pos.y<0 || p.pos.z<0   )
	{
	p.age=newAge;
	p.vec.z=0;
	gstreakBuffer[n]=p;
		return;
    }
	if(newAge>0)
	{
	if(p.age<=0) ageDiff=newAge-0;
	if(p.age>0) ageDiff=newAge-p.age;

	p.pos=p.pos+d*ageDiff;
    p.age=newAge;
    gstreakBuffer[n]=p; 
	}	

	if(newAge<=0)
	{ //newAge < 0
	p.age=newAge;
	gstreakBuffer[n]=p;
	}	
}

[numthreads(512,1,1)] //512 threads per block (most unoptimized code i ever wrote, although it works fine it is still shameful)
void OpacityCS(uint3 threadID : SV_DispatchThreadID, uint3 groupID : SV_GroupID,uint threadIDInGroup : SV_GroupIndex)
{
	int id=threadID.x;
	Particle p,p1,p2,p3,p4,p5,p6;
	float3 normal1,normal2,normal3,normal4,normal5,normal6;
	float3 viewingDirection;
	float viewingLength,cos1,cos2,cos3,cos4,cos5,cos6,opacity1,opacity2,opacity3,opacity4,opacity5,opacity6,shape1,shape2,shape3,shape4,shape5,shape6;
	float d0,d1,d2,area,opacityDensity,opacityShape,opacityFade;
	float k=0.2;
	float s=0.7;
	float maxLength=0.3;
	if(id%streakLinesLength==0)
	{
		//first vertex in a line

		p=gstreakBuffer[id];
		p3=gstreakBuffer[id-streakLinesLength];
		p4=gstreakBuffer[id-streakLinesLength-1];
		p5=gstreakBuffer[id-1];
		p6=gstreakBuffer[id+streakLinesLength];
	
		normal3=normalize(cross(p3.pos-p.pos,p4.pos-p.pos));
		normal4=normalize(cross(p4.pos-p.pos,p5.pos-p.pos));
		normal5=normalize(cross(p5.pos-p.pos,p6.pos-p.pos));	

		
		viewingDirection=p.pos-CameraAt;
		viewingLength=length(viewingDirection);
		cos3=dot(normal3,viewingDirection)/(length(normal3)*viewingLength);
		cos4=dot(normal4,viewingDirection)/(length(normal4)*viewingLength);
		cos5=dot(normal5,viewingDirection)/(length(normal5)*viewingLength);

		area=length(cross(p4.pos-p.pos,p3.pos-p.pos))*0.5;
		opacity3=abs(k/(area*cos3));
		d0=length(p4.pos-p.pos);
		d1=length(p3.pos-p.pos);
		d2=length(p4.pos-p3.pos);
		shape3=pow((4*area)/(sqrt(3)*max(max(d0*d1,d1*d2),d2*d0)),s);
		if(d0>maxLength || d1>maxLength || d2>maxLength) {
		shape1=0;opacity1=0;
		}	

		area=length(cross(p5.pos-p.pos,p4.pos-p.pos))*0.5;
		opacity4=abs(k/(area*cos4));
		d0=length(p5.pos-p.pos);
		d1=length(p4.pos-p.pos);
		d2=length(p5.pos-p4.pos);
		shape4=pow((4*area)/(sqrt(3)*max(max(d0*d1,d1*d2),d2*d0)),s);
		if(d0>maxLength || d1>maxLength || d2>maxLength) {
		shape1=0;opacity1=0;
		}

		area=length(cross(p6.pos-p.pos,p5.pos-p.pos))*0.5;
		opacity5=abs(k/(area*cos5));
		d0=length(p6.pos-p.pos);
		d1=length(p5.pos-p.pos);
		d2=length(p6.pos-p5.pos);
		float shape5=pow((4*area)/(sqrt(3)*max(max(d0*d1,d1*d2),d2*d0)),s);
		if(d0>maxLength || d1>maxLength || d2>maxLength) {
		shape1=0;opacity1=0;
		}
		opacityDensity = min(min(opacity3,opacity4),opacity5);
		opacityShape	 = min(min(shape3,shape4),shape5);
		opacityDensity=clamp(opacityDensity,0,1);
		opacityFade=1-(p.age/lifetime);
		if(d0>maxLength || d1>maxLength || d2>maxLength) {
		shape1=0;opacity1=0;
		}

		if(!useDensity) opacityDensity=1;
		if(!useShape) opacityShape=1;
		if(!useFade) opacityFade=1;

	

		p.vec.z=opacityDensity*opacityShape*opacityFade;

		gstreakBuffer[id]=p;

		
		return;
	}
	if(id%streakLinesLength==streakLinesLength-1)
	{
		//last vertex in a line
		p=gstreakBuffer[id];
		p3=gstreakBuffer[id-streakLinesLength-1];
		p4=gstreakBuffer[id-1];
		p5=gstreakBuffer[id+streakLinesLength];
		p6=gstreakBuffer[id-streakLinesLength];
	
		normal3=normalize(cross(p3.pos-p.pos,p4.pos-p.pos));
		normal4=normalize(cross(p4.pos-p.pos,p5.pos-p.pos));
		normal5=normalize(cross(p5.pos-p.pos,p6.pos-p.pos));	

		
		viewingDirection=p.pos-CameraAt;
		viewingLength=length(viewingDirection);
		cos3=dot(normal3,viewingDirection)/(length(normal3)*viewingLength);
		cos4=dot(normal4,viewingDirection)/(length(normal4)*viewingLength);
		cos5=dot(normal5,viewingDirection)/(length(normal5)*viewingLength);

		area=length(cross(p4.pos-p.pos,p3.pos-p.pos))*0.5;
		opacity3=abs(k/(area*cos3));
		d0=length(p4.pos-p.pos);
		d1=length(p3.pos-p.pos);
		d2=length(p4.pos-p3.pos);
		shape3=pow((4*area)/(sqrt(3)*max(max(d0*d1,d1*d2),d2*d0)),s);
		if(d0>maxLength || d1>maxLength || d2>maxLength) {
		shape1=0;opacity1=0;
		}	

		area=length(cross(p5.pos-p.pos,p4.pos-p.pos))*0.5;
		opacity4=abs(k/(area*cos4));
		d0=length(p5.pos-p.pos);
		d1=length(p4.pos-p.pos);
		d2=length(p5.pos-p4.pos);
		shape4=pow((4*area)/(sqrt(3)*max(max(d0*d1,d1*d2),d2*d0)),s);
		if(d0>maxLength || d1>maxLength || d2>maxLength) {
		shape1=0;opacity1=0;
		}

		area=length(cross(p6.pos-p.pos,p5.pos-p.pos))*0.5;
		opacity5=abs(k/(area*cos5));
		d0=length(p6.pos-p.pos);
		d1=length(p5.pos-p.pos);
		d2=length(p6.pos-p5.pos);
		float shape5=pow((4*area)/(sqrt(3)*max(max(d0*d1,d1*d2),d2*d0)),s);
		if(d0>maxLength || d1>maxLength || d2>maxLength) {
		shape1=0;opacity1=0;
		}
		opacityDensity = min(min(opacity3,opacity4),opacity5);
		opacityShape	 = min(min(shape3,shape4),shape5);
		opacityDensity=clamp(opacityDensity,0,1);
		opacityFade=1-(p.age/lifetime);
		if(d0>maxLength || d1>maxLength || d2>maxLength) {
		shape1=0;opacity1=0;
		}

		if(!useDensity) opacityDensity=1;
		if(!useShape) opacityShape=1;
		if(!useFade) opacityFade=1;

	

		p.vec.z=opacityDensity*opacityShape*opacityFade;

		gstreakBuffer[id]=p;

		return;
	}

	if(id<streakLinesLength)
	{
		//first line
		p=gstreakBuffer[id];
		p3=gstreakBuffer[id-1];
		p4=gstreakBuffer[id+streakLinesLength];
		p5=gstreakBuffer[id+streakLinesLength+1];
		p6=gstreakBuffer[id+1];
	
		normal3=normalize(cross(p3.pos-p.pos,p4.pos-p.pos));
		normal4=normalize(cross(p4.pos-p.pos,p5.pos-p.pos));
		normal5=normalize(cross(p5.pos-p.pos,p6.pos-p.pos));	

		
		viewingDirection=p.pos-CameraAt;
		viewingLength=length(viewingDirection);
		cos3=dot(normal3,viewingDirection)/(length(normal3)*viewingLength);
		cos4=dot(normal4,viewingDirection)/(length(normal4)*viewingLength);
		cos5=dot(normal5,viewingDirection)/(length(normal5)*viewingLength);

		area=length(cross(p4.pos-p.pos,p3.pos-p.pos))*0.5;
		opacity3=abs(k/(area*cos3));
		d0=length(p4.pos-p.pos);
		d1=length(p3.pos-p.pos);
		d2=length(p4.pos-p3.pos);
		shape3=pow((4*area)/(sqrt(3)*max(max(d0*d1,d1*d2),d2*d0)),s);
		if(d0>maxLength || d1>maxLength || d2>maxLength) {
		shape1=0;opacity1=0;
		}	

		area=length(cross(p5.pos-p.pos,p4.pos-p.pos))*0.5;
		opacity4=abs(k/(area*cos4));
		d0=length(p5.pos-p.pos);
		d1=length(p4.pos-p.pos);
		d2=length(p5.pos-p4.pos);
		shape4=pow((4*area)/(sqrt(3)*max(max(d0*d1,d1*d2),d2*d0)),s);
		if(d0>maxLength || d1>maxLength || d2>maxLength) {
		shape1=0;opacity1=0;
		}

		area=length(cross(p6.pos-p.pos,p5.pos-p.pos))*0.5;
		opacity5=abs(k/(area*cos5));
		d0=length(p6.pos-p.pos);
		d1=length(p5.pos-p.pos);
		d2=length(p6.pos-p5.pos);
		float shape5=pow((4*area)/(sqrt(3)*max(max(d0*d1,d1*d2),d2*d0)),s);
		if(d0>maxLength || d1>maxLength || d2>maxLength) {
		shape1=0;opacity1=0;
		}
		opacityDensity = min(min(opacity3,opacity4),opacity5);
		opacityShape	 = min(min(shape3,shape4),shape5);
		opacityDensity=clamp(opacityDensity,0,1);
		opacityFade=1-(p.age/lifetime);
		if(d0>maxLength || d1>maxLength || d2>maxLength) {
		shape1=0;opacity1=0;
		}

		if(!useDensity) opacityDensity=1;
		if(!useShape) opacityShape=1;
		if(!useFade) opacityFade=1;

	

		p.vec.z=opacityDensity*opacityShape*opacityFade;

		gstreakBuffer[id]=p;

		
		return;
	}

	if(id>=(numberOfLines-1)*streakLinesLength)
	{
		//last line

		p=gstreakBuffer[id];
		p3=gstreakBuffer[id+1];
		p4=gstreakBuffer[id-streakLinesLength];
		p5=gstreakBuffer[id-streakLinesLength-1];
		p6=gstreakBuffer[id-1];
	
		normal3=normalize(cross(p3.pos-p.pos,p4.pos-p.pos));
		normal4=normalize(cross(p4.pos-p.pos,p5.pos-p.pos));
		normal5=normalize(cross(p5.pos-p.pos,p6.pos-p.pos));	

		
		viewingDirection=p.pos-CameraAt;
		viewingLength=length(viewingDirection);
		cos3=dot(normal3,viewingDirection)/(length(normal3)*viewingLength);
		cos4=dot(normal4,viewingDirection)/(length(normal4)*viewingLength);
		cos5=dot(normal5,viewingDirection)/(length(normal5)*viewingLength);

		area=length(cross(p4.pos-p.pos,p3.pos-p.pos))*0.5;
		opacity3=abs(k/(area*cos3));
		d0=length(p4.pos-p.pos);
		d1=length(p3.pos-p.pos);
		d2=length(p4.pos-p3.pos);
		shape3=pow((4*area)/(sqrt(3)*max(max(d0*d1,d1*d2),d2*d0)),s);
		if(d0>maxLength || d1>maxLength || d2>maxLength) {
		shape1=0;opacity1=0;
		}	

		area=length(cross(p5.pos-p.pos,p4.pos-p.pos))*0.5;
		opacity4=abs(k/(area*cos4));
		d0=length(p5.pos-p.pos);
		d1=length(p4.pos-p.pos);
		d2=length(p5.pos-p4.pos);
		shape4=pow((4*area)/(sqrt(3)*max(max(d0*d1,d1*d2),d2*d0)),s);
		if(d0>maxLength || d1>maxLength || d2>maxLength) {
		shape1=0;opacity1=0;
		}

		area=length(cross(p6.pos-p.pos,p5.pos-p.pos))*0.5;
		opacity5=abs(k/(area*cos5));
		d0=length(p6.pos-p.pos);
		d1=length(p5.pos-p.pos);
		d2=length(p6.pos-p5.pos);
		float shape5=pow((4*area)/(sqrt(3)*max(max(d0*d1,d1*d2),d2*d0)),s);
		if(d0>maxLength || d1>maxLength || d2>maxLength) {
		shape1=0;opacity1=0;
		}
		opacityDensity = min(min(opacity3,opacity4),opacity5);
		opacityShape	 = min(min(shape3,shape4),shape5);
		opacityDensity=clamp(opacityDensity,0,1);
		opacityFade=1-(p.age/lifetime);
		if(d0>maxLength || d1>maxLength || d2>maxLength) {
		shape1=0;opacity1=0;
		}

		if(!useDensity) opacityDensity=1;
		if(!useShape) opacityShape=1;
		if(!useFade) opacityFade=1;

	

		p.vec.z=opacityDensity*opacityShape*opacityFade;

		gstreakBuffer[id]=p;

		
		return;

	}


	//general case
	p=gstreakBuffer[id];
	p1=gstreakBuffer[id-streakLinesLength];
	p2=gstreakBuffer[id-streakLinesLength-1];
	p3=gstreakBuffer[id-1];
	p4=gstreakBuffer[id+streakLinesLength];
	p5=gstreakBuffer[id+streakLinesLength+1];
	p6=gstreakBuffer[id+1];

	viewingDirection=p.pos-CameraAt;
	viewingLength=length(viewingDirection);
	//normals
	normal1=normalize(cross(p1.pos-p.pos,p2.pos-p1.pos));
	normal2=normalize(cross(p2.pos-p.pos,p3.pos-p2.pos));
	normal3=normalize(cross(p3.pos-p.pos,p4.pos-p3.pos));
	normal4=normalize(cross(p4.pos-p.pos,p5.pos-p4.pos));
	normal5=normalize(cross(p5.pos-p.pos,p6.pos-p5.pos));
	normal6=normalize(cross(p6.pos-p.pos,p1.pos-p6.pos));

	//cosines
	cos1=dot(normal1,viewingDirection)/(length(normal1)*viewingLength);
	cos2=dot(normal2,viewingDirection)/(length(normal2)*viewingLength);
	cos3=dot(normal3,viewingDirection)/(length(normal3)*viewingLength);
	cos4=dot(normal4,viewingDirection)/(length(normal4)*viewingLength);
	cos5=dot(normal5,viewingDirection)/(length(normal5)*viewingLength);
	cos6=dot(normal1,viewingDirection)/(length(normal6)*viewingLength);
	
	//float area=sin(angle)*( length(p2.pos-p.pos)*length(p1.pos-p.pos) )*0.5;
	area=length(cross(p2.pos-p.pos,p1.pos-p.pos))*0.5;
	opacity1=abs(k/(area*cos1));
	d0=length(p2.pos-p.pos);
	d1=length(p1.pos-p.pos);
	d2=length(p2.pos-p1.pos);
	shape1=pow((4*area)/(sqrt(3)*max(max(d0*d1,d1*d2),d2*d0)),s);
	if(d0>maxLength || d1>maxLength || d2>maxLength) 
	{
	shape1=0;opacity1=0;
	}

	area=length(cross(p3.pos-p.pos,p2.pos-p.pos))*0.5;
	opacity2=abs(k/(area*cos2));
	d0=length(p3.pos-p.pos);
	d1=length(p2.pos-p.pos);
	d2=length(p3.pos-p2.pos);
	shape2=pow((4*area)/(sqrt(3)*max(max(d0*d1,d1*d2),d2*d0)),s);
	if(d0>maxLength || d1>maxLength || d2>maxLength) {
	shape1=0;opacity1=0;
	}

	area=length(cross(p4.pos-p.pos,p3.pos-p.pos))*0.5;
	opacity3=abs(k/(area*cos3));
	d0=length(p4.pos-p.pos);
	d1=length(p3.pos-p.pos);
	d2=length(p4.pos-p3.pos);
	shape3=pow((4*area)/(sqrt(3)*max(max(d0*d1,d1*d2),d2*d0)),s);
	if(d0>maxLength || d1>maxLength || d2>maxLength) {
	shape1=0;opacity1=0;
	}

	area=length(cross(p5.pos-p.pos,p4.pos-p.pos))*0.5;
	opacity4=abs(k/(area*cos4));
	d0=length(p5.pos-p.pos);
	d1=length(p4.pos-p.pos);
	d2=length(p5.pos-p4.pos);
	shape4=pow((4*area)/(sqrt(3)*max(max(d0*d1,d1*d2),d2*d0)),s);
	if(d0>maxLength || d1>maxLength || d2>maxLength) {
	shape1=0;opacity1=0;
	}

	area=length(cross(p6.pos-p.pos,p5.pos-p.pos))*0.5;
	opacity5=abs(k/(area*cos5));
	d0=length(p6.pos-p.pos);
	d1=length(p5.pos-p.pos);
	d2=length(p6.pos-p5.pos);
	shape5=pow((4*area)/(sqrt(3)*max(max(d0*d1,d1*d2),d2*d0)),s);
	if(d0>maxLength || d1>maxLength || d2>maxLength) {
	shape1=0;opacity1=0;
	}

	area=length(cross(p1.pos-p.pos,p6.pos-p.pos))*0.5;
	opacity6=abs(k/(area*cos6));
	d0=length(p1.pos-p.pos);
	d1=length(p6.pos-p.pos);
	d2=length(p1.pos-p6.pos);
	shape6=pow((4*area)/(sqrt(3)*max(max(d0*d1,d1*d2),d2*d0)),s);
	if(d0>maxLength || d1>maxLength || d2>maxLength) {
	shape1=0;opacity1=0;
	}

	opacityDensity = min(min(min(min(min(opacity1,opacity2),opacity3),opacity4),opacity5),opacity6);
	opacityShape	 = min(min(min(min(min(shape1,shape2),shape3),shape4),shape5),shape6);
	opacityDensity=clamp(opacityDensity,0,1);
	opacityFade=1-(p.age/lifetime);
	if(d0>maxLength || d1>maxLength || d2>maxLength) {
	shape1=0;opacity1=0;
	}

	if(!useDensity) opacityDensity=1;
	if(!useShape) opacityShape=1;
	if(!useFade) opacityFade=1;

	

	p.vec.z=opacityDensity*opacityShape*opacityFade;

	gstreakBuffer[id]=p;

}

technique11 Render
{
    pass Domain
	{
        SetVertexShader(CompileShader(vs_5_0, DomainVS()));
        SetPixelShader(CompileShader(ps_5_0, DomainPS()));
        SetGeometryShader( NULL );
        SetRasterizerState(rsCullBack);
        SetDepthStencilState(dsEnableDepth, 0);
        SetBlendState(bsNoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }


	pass SimpleVolumeDVR
	{
        SetVertexShader(CompileShader(vs_5_0, VolumeVS()));
        SetPixelShader(CompileShader(ps_5_0, VolumePSDVR()));
        SetGeometryShader( NULL );
        SetRasterizerState(rsCullNone);
        SetDepthStencilState(dsDisableDepth, 0);
        SetBlendState(bsBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }


	pass SimpleVolumeISO
	{
        SetVertexShader(CompileShader(vs_5_0, VolumeVS()));
        SetPixelShader(CompileShader(ps_5_0, VolumePSISO()));
        SetGeometryShader( NULL );
        SetRasterizerState(rsCullNone);
        SetDepthStencilState(dsDisableDepth, 0);
        SetBlendState(bsNoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }


	pass Probes
	{
        SetVertexShader(CompileShader(vs_5_0, ProbeVS()));
        SetPixelShader(CompileShader(ps_5_0, ProbePS()));
        SetGeometryShader( NULL );
        SetComputeShader(CompileShader(cs_5_0, ParticlesCS()));
        SetRasterizerState(rsCullBack);
        SetDepthStencilState(dsEnableDepth, 0);
        SetBlendState(bsNoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }


	pass Arrows
	{
        SetVertexShader(CompileShader(vs_5_0, ArrowsVS()));
        SetPixelShader(CompileShader(ps_5_0, ArrowsPS()));
        SetGeometryShader(CompileShader(gs_5_0, ArrowsGS()) );
        SetRasterizerState(rsCullNone);
        SetDepthStencilState(dsEnableDepth, 0);
        SetBlendState(bsNoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }


   pass Particles
   {
        SetVertexShader(CompileShader(vs_5_0, ParticlesVS()));
        SetPixelShader(CompileShader(ps_5_0, PointsPS()));
        SetGeometryShader(NULL);
        SetRasterizerState(rsCullNone);
        SetDepthStencilState(dsEnableDepth, 0);
        SetBlendState(bsNoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }


   pass VelocityMag
   {
        SetComputeShader(CompileShader(cs_5_0, VelocityMagCS()));
    }


   pass Divergence
   {
        SetComputeShader(CompileShader(cs_5_0, DivergenceCS()));
    }


    pass VorticityMag
   {
        SetComputeShader(CompileShader(cs_5_0, VorticityMagCS()));
    }


   pass Q
   {
        SetComputeShader(CompileShader(cs_5_0, QCS()));
    }
	pass Integrate
	{
		SetComputeShader(CompileShader(cs_5_0, StreamLinesCS()));
	}
	pass IntegrateRibbons
	{
		SetComputeShader(CompileShader(cs_5_0, RibbonsCS()));
	}
	pass IntegrateStreakLines
	{
		SetComputeShader(CompileShader(cs_5_0, StreakLinesCS()));
	}
	pass Opacity
	{
		SetComputeShader(CompileShader(cs_5_0, OpacityCS()));
	}
	pass StreakLines
	{
		SetVertexShader(CompileShader(vs_5_0, StreakLinesVS()));
        SetPixelShader(CompileShader(ps_5_0, StreakLinesPS()));
        SetGeometryShader(CompileShader(gs_5_0, StreakLinesGS()));
       
        SetRasterizerState(rsCullNone);
        SetDepthStencilState(dsEnableDepthDisableWrite, 0);
        SetBlendState(bsBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
	pass StreamLines
	{
		SetVertexShader(CompileShader(vs_5_0, StreamLinesVS()));
        SetPixelShader(CompileShader(ps_5_0, StreamLinesPS()));
        SetGeometryShader(CompileShader(gs_5_0, StreamLinesGS()));
       
        SetRasterizerState(rsCullNone);
        SetDepthStencilState(dsEnableDepth, 0);
        SetBlendState(bsNoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
	pass Ribbons
	{
		SetVertexShader(CompileShader(vs_5_0, RibbonsVS()));
        SetPixelShader(CompileShader(ps_5_0, RibbonsPS()));
        SetGeometryShader( NULL );
       
        SetRasterizerState(rsCullNone);
        SetDepthStencilState(dsEnableDepth, 0);
        SetBlendState(bsNoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
	pass MeshPass
	{
        SetVertexShader(CompileShader(vs_5_0, SimpleMeshVS()));
        SetPixelShader(CompileShader(ps_5_0, SimpleMeshPS()));
        SetGeometryShader( NULL );
        SetRasterizerState(rsCullNone);
        SetDepthStencilState(dsEnableDepth, 0);
        SetBlendState(bsNoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }


}