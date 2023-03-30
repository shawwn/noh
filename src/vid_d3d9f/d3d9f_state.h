// (C)2005 S2 Games
// d3d9f_state.h
//
// Direct3D State Manager
//=============================================================================
#ifndef __D3D9F_STATE_H__
#define __D3D9F_STATE_H__

const int NUM_RENDERSTATES = 209; // As of 9.0c
const int NUM_SAMPLERS = 16;
const int NUM_STREAMS = 16;
const int NUM_SAMPLERSTATES = 13;
const int NUM_TEXTURESTAGESTATES = 32;

extern DWORD		g_TopRenderStates[NUM_RENDERSTATES];
extern DWORD		g_TopSamplerStates[NUM_SAMPLERS][NUM_SAMPLERSTATES];
extern DWORD		g_TopTextureStageStates[NUM_SAMPLERS][NUM_TEXTURESTAGESTATES];

extern IDirect3DBaseTexture9*		g_TopTexture[NUM_SAMPLERS];
extern IDirect3DIndexBuffer9*		g_TopIndices;
extern IDirect3DVertexBuffer9*		g_TopStreamSource[NUM_STREAMS];
extern IDirect3DVertexShader9*		g_TopVertexShader;
extern IDirect3DPixelShader9*		g_TopPixelShader;
extern IDirect3DVertexDeclaration9*	g_TopVertexDeclaration;

extern vector<DWORD>		g_RenderStates[NUM_RENDERSTATES];
extern vector<DWORD>		g_SamplerStates[NUM_SAMPLERS][NUM_SAMPLERSTATES];
extern vector<DWORD>		g_TextureStageStates[NUM_SAMPLERS][NUM_TEXTURESTAGESTATES];

extern vector<IDirect3DBaseTexture9 *>			g_Texture[NUM_SAMPLERS];
extern vector<IDirect3DIndexBuffer9 *>			g_Indices;
extern vector<IDirect3DVertexBuffer9 *>			g_StreamSource[NUM_STREAMS];
extern vector<IDirect3DVertexShader9 *>			g_VertexShader;
extern vector<IDirect3DPixelShader9 *>			g_PixelShader;
extern vector<IDirect3DVertexDeclaration9 *>	g_VertexDeclaration;

bool	D3D_SetRenderStateDefault(D3DRENDERSTATETYPE State, DWORD Value);
bool	D3D_SetRenderState(D3DRENDERSTATETYPE State, DWORD Value);
DWORD	D3D_GetRenderState(D3DRENDERSTATETYPE State);
bool	D3D_PushRenderState(D3DRENDERSTATETYPE State, DWORD Value);
bool	D3D_PushRenderState(D3DRENDERSTATETYPE State);
bool	D3D_PopRenderState(D3DRENDERSTATETYPE State);

bool	D3D_SetSamplerStateDefault(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
bool	D3D_SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
DWORD	D3D_GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type);
bool	D3D_PushSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
bool	D3D_PushSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type);
bool	D3D_PopSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type);

bool	D3D_SetTextureStageStateDefault(DWORD Sampler, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
bool	D3D_SetTextureStageState(DWORD Sampler, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
DWORD	D3D_GetTextureStageState(DWORD Sampler, D3DTEXTURESTAGESTATETYPE Type);
bool	D3D_PushTextureStageState(DWORD Sampler, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
bool	D3D_PushTextureStageState(DWORD Sampler, D3DTEXTURESTAGESTATETYPE Type);
bool	D3D_PopTextureStageState(DWORD Sampler, D3DTEXTURESTAGESTATETYPE Type);

bool	D3D_SetTextureDefault(DWORD Sampler, IDirect3DBaseTexture9 *pTexture);
bool	D3D_SetTexture(DWORD Sampler, IDirect3DBaseTexture9 *pTexture);
IDirect3DBaseTexture9*	D3D_GetTexture(DWORD Sampler);
bool	D3D_PushTexture(DWORD Sampler, IDirect3DBaseTexture9 *pTexture);
bool	D3D_PushTexture(DWORD Sampler);
bool	D3D_PopTexture(DWORD Sampler);

bool	D3D_SetIndicesDefault(IDirect3DIndexBuffer9 *pIndices);
bool	D3D_SetIndices(IDirect3DIndexBuffer9 *pIndices);
IDirect3DIndexBuffer9*	D3D_GetIndices();
bool	D3D_PushIndices(IDirect3DIndexBuffer9 *pIndices);
bool	D3D_PushIndices();
bool	D3D_PopIndices();

bool	D3D_SetStreamSourceDefault(UINT StreamNumber, IDirect3DVertexBuffer9 *pStreamData, UINT OffsetInBytes, UINT Stride);
bool	D3D_SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9 *pStreamData, UINT OffsetInBytes, UINT Stride);
IDirect3DVertexBuffer9*	D3D_GetStreamSource(UINT StreamNumber);
bool	D3D_PushStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9 *pStreamData, UINT OffsetInBytes, UINT Stride);
bool	D3D_PushStreamSource(UINT StreamNumber);
bool	D3D_PopStreamSource(UINT StreamNumber);

bool	D3D_SetVertexDeclarationDefault(IDirect3DVertexDeclaration9 *pDecl);
bool	D3D_SetVertexDeclaration(IDirect3DVertexDeclaration9 *pDecl);
IDirect3DVertexDeclaration9*	D3D_GetVertexDeclaration();
bool	D3D_PushVertexDeclaration(IDirect3DVertexDeclaration9 *pDecl);
bool	D3D_PushVertexDeclaration();
bool	D3D_PopVertexDeclaration();


/*====================
  D3D_SetRenderState
  ====================*/
inline
bool	D3D_SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	int i(State - 1); // D3DRENDERSTATETYPE is 1 based, so convert to 0 based

	assert(i >= 0 && i < NUM_RENDERSTATES);

	// Inform D3D API if the state actually changes
	if (g_TopRenderStates[i] != Value)
	{
		PROFILE("RenderState Change");

		// Change the top of the stack to the new value
		g_TopRenderStates[i] = Value;

		g_pd3dDevice->SetRenderState(State, Value);
		return true;
	}

	return false;
}


/*====================
  D3D_SetSamplerState
  ====================*/
inline
bool	D3D_SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	int i(Type - 1); // D3DSAMPLERSTATETYPE is 1 based, so convert to 0 based

	assert(Sampler >= 0 && Sampler < NUM_SAMPLERS);
	assert(i >= 0 && i < NUM_SAMPLERSTATES);

	// Inform D3D API if the state actually changes
	if (g_TopSamplerStates[Sampler][i] != Value)
	{
		PROFILE("SamplerState Change");

		// Change the top of the stack to the new value
		g_TopSamplerStates[Sampler][i] = Value;

		g_pd3dDevice->SetSamplerState(Sampler, Type, Value);
		return true;
	}

	return false;
}


/*====================
  D3D_SetTextureStageState
  ====================*/
inline
bool	D3D_SetTextureStageState(DWORD Sampler, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	int i = Type - 1; // D3DTEXTURESTAGESTATETYPE is 1 based, so convert to 0 based

	assert(Sampler >= 0 && Sampler < NUM_SAMPLERS);
	assert(i >= 0 && i < NUM_TEXTURESTAGESTATES);

	// Inform D3D API if the state actually changes
	if (g_TopTextureStageStates[Sampler][i] != Value)
	{
		// Change the top of the stack to the new value
		g_TopTextureStageStates[Sampler][i] = Value;

		g_pd3dDevice->SetTextureStageState(Sampler, Type, Value);
		return true;
	}

	return false;
}


/*====================
  D3D_SetTexture
  ====================*/
inline
bool	D3D_SetTexture(DWORD Sampler, IDirect3DBaseTexture9 *pTexture)
{
	assert(Sampler >= 0 && Sampler < NUM_SAMPLERS);

	// Inform D3D API if the state actually changes
	if (g_TopTexture[Sampler] != pTexture)
	{
		// Change the top of the stack to the new texture
		g_TopTexture[Sampler] = pTexture;

		g_pd3dDevice->SetTexture(Sampler, pTexture);
		return true;
	}

	return false;
}


/*====================
  D3D_SetIndices
  ====================*/
inline
bool	D3D_SetIndices(IDirect3DIndexBuffer9 *pIndices)
{
	// Inform D3D API if the state actually changes
	if (g_TopIndices != pIndices)
	{
		// Change the top of the stack to the new texture
		g_TopIndices = pIndices;

		g_pd3dDevice->SetIndices(pIndices);

		return true;
	}

	return false;
}


/*====================
  D3D_SetStreamSource
  ====================*/
inline
bool	D3D_SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9 *pStreamData, UINT OffsetInBytes, UINT Stride)
{
	assert(StreamNumber >= 0 && StreamNumber < NUM_STREAMS);

	// Inform D3D API if the state actually changes
	if (g_TopStreamSource[StreamNumber] != pStreamData)
	{
		// Change the top of the stack to the new texture
		g_TopStreamSource[StreamNumber] = pStreamData;

		g_pd3dDevice->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);

		return true;
	}

	return false;
}


/*====================
  D3D_SetVertexDeclaration
  ====================*/
inline
bool	D3D_SetVertexDeclaration(IDirect3DVertexDeclaration9 *pDecl)
{
	// Inform D3D API if the state actually changes
	if (g_TopVertexDeclaration != pDecl)
	{
		PROFILE("VertexDeclaration Change");

		// Change the top of the stack to the new value
		g_TopVertexDeclaration = pDecl;

		g_pd3dDevice->SetVertexDeclaration(pDecl);

		return true;
	}

	return false;
}


#endif // __D3D9F_STATE_H__