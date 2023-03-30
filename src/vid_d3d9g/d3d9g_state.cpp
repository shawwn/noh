// (C)2005 S2 Games
// d3d9g_state.cpp
//
// Direct3D state functions
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "d3d9g_main.h"
#include "d3d9g_state.h"
//=============================================================================

DWORD		g_TopRenderStates[NUM_RENDERSTATES];
DWORD		g_TopSamplerStates[NUM_SAMPLERS][NUM_SAMPLERSTATES];
DWORD		g_TopTextureStageStates[NUM_SAMPLERS][NUM_TEXTURESTAGESTATES];

IDirect3DBaseTexture9*		g_TopTexture[NUM_SAMPLERS];
IDirect3DIndexBuffer9*		g_TopIndices;
IDirect3DVertexBuffer9*		g_TopStreamSource[NUM_STREAMS];
IDirect3DVertexShader9*		g_TopVertexShader;
IDirect3DPixelShader9*		g_TopPixelShader;
IDirect3DVertexDeclaration9*	g_TopVertexDeclaration;

vector<DWORD>		g_RenderStates[NUM_RENDERSTATES];
vector<DWORD>		g_SamplerStates[NUM_SAMPLERS][NUM_SAMPLERSTATES];
vector<DWORD>		g_TextureStageStates[NUM_SAMPLERS][NUM_TEXTURESTAGESTATES];

vector<IDirect3DBaseTexture9 *>			g_Texture[NUM_SAMPLERS];
vector<IDirect3DIndexBuffer9 *>			g_Indices;
vector<IDirect3DVertexBuffer9 *>		g_StreamSource[NUM_STREAMS];
vector<IDirect3DVertexShader9 *>		g_VertexShader;
vector<IDirect3DPixelShader9 *>			g_PixelShader;
vector<IDirect3DVertexDeclaration9 *>	g_VertexDeclaration;

/*====================
  D3D_SetRenderStateDefault
  ====================*/
bool	D3D_SetRenderStateDefault(D3DRENDERSTATETYPE State, DWORD Value)
{
	int i(State - 1); // D3DRENDERSTATETYPE is 1 based, so convert to 0 based

	assert(i >= 0 && i < NUM_RENDERSTATES);

	// Clear the stack
	g_RenderStates[i].clear();

	// Push the default state as the bottom element of the stack
    g_TopRenderStates[i] = Value;

	// Actually set the render state
	g_pd3dDevice->SetRenderState(State, Value);
	return true;
}


/*====================
  D3D_GetRenderState
  ====================*/
DWORD	D3D_GetRenderState(D3DRENDERSTATETYPE State)
{
	int i(State - 1); // D3DRENDERSTATETYPE is 1 based, so convert to 0 based

	assert(i >= 0 && i < NUM_RENDERSTATES);

	return g_TopRenderStates[i];
}


/*====================
  D3D_PushRenderState
  ====================*/
bool	D3D_PushRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	int i(State - 1); // D3DRENDERSTATETYPE is 1 based, so convert to 0 based

	assert(i >= 0 && i < NUM_RENDERSTATES);
	assert(g_RenderStates[i].size() < 256);

	DWORD dwCurrentValue(g_TopRenderStates[i]);

	// Push the old state onto the stack
    g_RenderStates[i].push_back(g_TopRenderStates[i]);
	
	g_TopRenderStates[i] = Value;

	// Inform D3D API if the state actually changes
	if (dwCurrentValue != Value)
	{
		g_pd3dDevice->SetRenderState(State, Value);
		return true;
	}

	return false;
}


/*====================
  D3D_PushRenderState
  ====================*/
bool	D3D_PushRenderState(D3DRENDERSTATETYPE State)
{
	int i(State - 1); // D3DRENDERSTATETYPE is 1 based, so convert to 0 based

	assert(i >= 0 && i < NUM_RENDERSTATES);
	assert(g_RenderStates[i].size() < 256);

	// Push a duplicate state onto the stack
    g_RenderStates[i].push_back(g_TopRenderStates[i]);

	return false;
}


/*====================
  D3D_PopRenderState
  ====================*/
bool	D3D_PopRenderState(D3DRENDERSTATETYPE State)
{
	int i(State - 1); // D3DRENDERSTATETYPE is 1 based, so convert to 0 based

	assert(i >= 0 && i < NUM_RENDERSTATES);

	DWORD dwOldValue(g_TopRenderStates[i]);

	// Pop the old state off the stack
	g_TopRenderStates[i] = g_RenderStates[i].back();
	g_RenderStates[i].pop_back();

	// Inform D3D API if the state actually changes
	if (dwOldValue != g_TopRenderStates[i])
	{
		g_pd3dDevice->SetRenderState(State, g_TopRenderStates[i]);
		return true;
	}

	return false;
}


/*====================
  D3D_SetSamplerStateDefault
  ====================*/
bool	D3D_SetSamplerStateDefault(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	int i(Type - 1); // D3DSAMPLERSTATETYPE is 1 based, so convert to 0 based

	assert(Sampler >= 0 && Sampler < NUM_SAMPLERS);
	assert(i >= 0 && i < NUM_SAMPLERSTATES);

	// Clear the stack
	g_SamplerStates[Sampler][i].clear();

	// Push the default state as the bottom element of the stack
    g_TopSamplerStates[Sampler][i] = Value;

	// Actually set the sampler state
	g_pd3dDevice->SetSamplerState(Sampler, Type, Value);

	return true;
}


/*====================
  D3D_GetSamplerState
  ====================*/
DWORD	D3D_GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type)
{
	int i(Type - 1); // D3DSAMPLERSTATETYPE is 1 based, so convert to 0 based

	assert(Sampler >= 0 && Sampler < NUM_SAMPLERS);
	assert(i >= 0 && i < NUM_SAMPLERSTATES);

	return g_TopSamplerStates[Sampler][i];
}


/*====================
  D3D_PushSamplerState
  ====================*/
bool	D3D_PushSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	int i(Type - 1); // D3DSAMPLERSTATETYPE is 1 based, so convert to 0 based

	assert(Sampler >= 0 && Sampler < NUM_SAMPLERS);
	assert(i >= 0 && i < NUM_SAMPLERSTATES);
	assert(g_SamplerStates[Sampler][i].size() < 256);

	DWORD dwCurrentValue(g_TopSamplerStates[Sampler][i]);

	// Push the old state onto the stack
    g_SamplerStates[Sampler][i].push_back(g_TopSamplerStates[Sampler][i]);

	g_TopSamplerStates[Sampler][i] = Value;

	// Inform D3D API if the state actually changes
	if (dwCurrentValue != Value)
	{
		g_pd3dDevice->SetSamplerState(Sampler, Type, Value);
		return true;
	}

	return false;
}


/*====================
  D3D_PushSamplerState
  ====================*/
bool	D3D_PushSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type)
{
	int i(Type - 1); // D3DSAMPLERSTATETYPE is 1 based, so convert to 0 based

	assert(Sampler >= 0 && Sampler < NUM_SAMPLERS);
	assert(i >= 0 && i < NUM_SAMPLERSTATES);
	assert(g_SamplerStates[Sampler][i].size() < 256);

	// Push a duplicate state onto the stack
    g_SamplerStates[Sampler][i].push_back(g_TopSamplerStates[Sampler][i]);

	return false;
}


/*====================
  D3D_PopSamplerState
  ====================*/
bool	D3D_PopSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type)
{
	int i(Type - 1); // D3DSAMPLERSTATETYPE is 1 based, so convert to 0 based

	assert(Sampler >= 0 && Sampler < NUM_SAMPLERS);
	assert(i >= 0 && i < NUM_SAMPLERSTATES);

	DWORD dwOldValue(g_TopSamplerStates[Sampler][i]);

	// Pop the old state off the stack
	g_TopSamplerStates[Sampler][i] = g_SamplerStates[Sampler][i].back();
	g_SamplerStates[Sampler][i].pop_back();

	// Inform D3D API if the state actually changes
	if (dwOldValue != g_TopSamplerStates[Sampler][i])
	{
		g_pd3dDevice->SetSamplerState(Sampler, Type, g_TopSamplerStates[Sampler][i]);
		return true;
	}

	return false;
}


/*====================
  D3D_SetTextureStageStateDefault
  ====================*/
bool	D3D_SetTextureStageStateDefault(DWORD Sampler, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	int i(Type - 1); // D3DTEXTURESTAGESTATETYPE is 1 based, so convert to 0 based

	assert(Sampler >= 0 && Sampler < NUM_SAMPLERS);
	assert(i >= 0 && i < NUM_TEXTURESTAGESTATES);

	// Clear the stack
	g_TextureStageStates[Sampler][i].clear();

	// Push the default state as the bottom element of the stack
	g_TopTextureStageStates[Sampler][i] = Value;

	// Actually set the sampler state
	g_pd3dDevice->SetTextureStageState(Sampler, Type, Value);
	return true;
}


/*====================
  D3D_GetTextureStageState
  ====================*/
DWORD	D3D_GetTextureStageState(DWORD Sampler, D3DTEXTURESTAGESTATETYPE Type)
{
	int i(Type - 1); // D3DTEXTURESTAGESTATETYPE is 1 based, so convert to 0 based

	assert(Sampler >= 0 && Sampler < NUM_SAMPLERS);
	assert(i >= 0 && i < NUM_TEXTURESTAGESTATES);

	return g_TopTextureStageStates[Sampler][i];
}


/*====================
  D3D_PushTextureStageState
  ====================*/
bool	D3D_PushTextureStageState(DWORD Sampler, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	int i(Type - 1); // D3DTEXTURESTAGESTATETYPE is 1 based, so convert to 0 based

	assert(Sampler >= 0 && Sampler < NUM_SAMPLERS);
	assert(i >= 0 && i < NUM_TEXTURESTAGESTATES);
	assert(g_TextureStageStates[Sampler][i].size() < 256);

	DWORD dwCurrentValue(g_TopTextureStageStates[Sampler][i]);

	// Push the old state onto the stack
    g_TextureStageStates[Sampler][i].push_back(g_TopTextureStageStates[Sampler][i]);

	g_TopTextureStageStates[Sampler][i] = Value;

	// Inform D3D API if the state actually changes
	if (dwCurrentValue != Value)
	{
		g_pd3dDevice->SetTextureStageState(Sampler, Type, Value);
		return true;
	}

	return false;
}


/*====================
  D3D_PushTextureStageState
  ====================*/
bool	D3D_PushTextureStageState(DWORD Sampler, D3DTEXTURESTAGESTATETYPE Type)
{
	int i(Type - 1); // D3DTEXTURESTAGESTATETYPE is 1 based, so convert to 0 based

	assert(Sampler >= 0 && Sampler < NUM_SAMPLERS);
	assert(i >= 0 && i < NUM_TEXTURESTAGESTATES);
	assert(g_TextureStageStates[Sampler][i].size() < 256);

	// Push a duplicate state state onto the stack
    g_TextureStageStates[Sampler][i].push_back(g_TopTextureStageStates[Sampler][i]);

	return false;
}


/*====================
  D3D_PopTextureStageState
  ====================*/
bool	D3D_PopTextureStageState(DWORD Sampler, D3DTEXTURESTAGESTATETYPE Type)
{
	int i(Type - 1); // D3DTEXTURESTAGESTATETYPE is 1 based, so convert to 0 based

	assert(Sampler >= 0 && Sampler < NUM_SAMPLERS);
	assert(i >= 0 && i < NUM_TEXTURESTAGESTATES);

	DWORD dwOldValue(g_TopTextureStageStates[Sampler][i]);

	// Pop the old state off the stack
	g_TopTextureStageStates[Sampler][i] = g_TextureStageStates[Sampler][i].back();
    g_TextureStageStates[Sampler][i].pop_back();

	// Inform D3D API if the state actually changes
	if (dwOldValue != g_TopTextureStageStates[Sampler][i])
	{
		g_pd3dDevice->SetTextureStageState(Sampler, Type, g_TopTextureStageStates[Sampler][i]);
		return true;
	}

	return false;
}


/*====================
  D3D_SetTextureDefault
  ====================*/
bool	D3D_SetTextureDefault(DWORD Sampler, IDirect3DBaseTexture9 *pTexture)
{
	assert(Sampler >= 0 && Sampler < NUM_SAMPLERS);

	// Clear the stack
	g_Texture[Sampler].clear();

	// Push the default state as the bottom element of the stack
    g_TopTexture[Sampler] = pTexture;

	// Actually set the texture
	g_pd3dDevice->SetTexture(Sampler, pTexture);
	return true;
}


/*====================
  D3D_GetTexture
  ====================*/
IDirect3DBaseTexture9*	D3D_GetTexture(DWORD Sampler)
{
	assert(Sampler >= 0 && Sampler < NUM_SAMPLERS);

	return g_TopTexture[Sampler];
}


/*====================
  D3D_PushTexture
  ====================*/
bool	D3D_PushTexture(DWORD Sampler, IDirect3DBaseTexture9 *pTexture)
{
	assert(Sampler >= 0 && Sampler < NUM_SAMPLERS);
	assert(g_Texture[Sampler].size() < 256);

	IDirect3DBaseTexture9 *pCurrentTexture(g_TopTexture[Sampler]);

	// Push the old state onto the stack
    g_Texture[Sampler].push_back(g_TopTexture[Sampler]);
	g_TopTexture[Sampler] = pTexture;

	// Inform D3D API if the state actually changes
	if (pCurrentTexture != pTexture)
	{
		g_pd3dDevice->SetTexture(Sampler, pTexture);
		return true;
	}

	return false;
}


/*====================
  D3D_PushTexture
  ====================*/
bool	D3D_PushTexture(DWORD Sampler)
{
	assert(Sampler >= 0 && Sampler < NUM_SAMPLERS);
	assert(g_Texture[Sampler].size() < 256);

	// Push a duplicate state onto the stack
    g_Texture[Sampler].push_back(g_TopTexture[Sampler]);

	return false;
}


/*====================
  D3D_PopTexture
  ====================*/
bool	D3D_PopTexture(DWORD Sampler)
{
	assert(Sampler >= 0 && Sampler < NUM_SAMPLERS);

	IDirect3DBaseTexture9 *pOldTexture(g_TopTexture[Sampler]);

	// Pop the old state off the stack
	g_TopTexture[Sampler] = g_Texture[Sampler].back();
    g_Texture[Sampler].pop_back();

	// Inform D3D API if the state actually changes
	if (pOldTexture != g_TopTexture[Sampler])
	{
		g_pd3dDevice->SetTexture(Sampler, g_TopTexture[Sampler]);
		return true;
	}

	return false;
}


/*====================
  D3D_SetIndicesDefault
  ====================*/
bool	D3D_SetIndicesDefault(IDirect3DIndexBuffer9 *pIndices)
{
	// Clear the stack
	g_Indices.clear();

	// Push the default state as the bottom element of the stack
	g_TopIndices = pIndices;

	// Actually set the texture
	g_pd3dDevice->SetIndices(pIndices);
	return true;
}


/*====================
  D3D_GetIndices
  ====================*/
IDirect3DIndexBuffer9*	D3D_GetIndices()
{
	return g_TopIndices;
}


/*====================
  D3D_PushIndices
  ====================*/
bool	D3D_PushIndices(IDirect3DIndexBuffer9 *pIndices)
{
	assert(g_Indices.size() < 256);

	IDirect3DIndexBuffer9 *pCurrentIndices = g_Indices.back();

	// Push the new state onto the stack
    g_Indices.push_back(pIndices);

	// Inform D3D API if the state actually changes
	if (pCurrentIndices != pIndices)
	{
		g_pd3dDevice->SetIndices(pIndices);
		return true;
	}

	return false;
}


/*====================
  D3D_PushIndices
  ====================*/
bool	D3D_PushIndices()
{
	assert(g_Indices.size() < 256);

	// Push a duplicate state onto the stack
    g_Indices.push_back(g_Indices.back());

	return false;
}


/*====================
  D3D_PopIndices
  ====================*/
bool	D3D_PopIndices()
{
	IDirect3DIndexBuffer9 *pOldIndices = g_Indices.back();

	// Pop the old state off the stack
    g_Indices.pop_back();

	IDirect3DIndexBuffer9 *pNewIndices = g_Indices.back();

	// Inform D3D API if the state actually changes
	if (pOldIndices != pNewIndices)
	{
		g_pd3dDevice->SetIndices(pNewIndices);
		return true;
	}

	return false;
}


/*====================
  D3D_SetStreamSourceDefault
  ====================*/
bool	D3D_SetStreamSourceDefault(UINT StreamNumber, IDirect3DVertexBuffer9 *pStreamData, UINT OffsetInBytes, UINT Stride)
{
	assert(StreamNumber >= 0 && StreamNumber < NUM_STREAMS);

	// Clear the stack
	g_StreamSource[StreamNumber].clear();

	// Push the default state as the bottom element of the stack
   g_TopStreamSource[StreamNumber] = pStreamData;

	// Actually set the texture
	g_pd3dDevice->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
	return true;
}


/*====================
  D3D_GetStreamSource
  ====================*/
IDirect3DVertexBuffer9*	D3D_GetStreamSource(UINT StreamNumber)
{
	assert(StreamNumber >= 0 && StreamNumber < NUM_STREAMS);

	return g_TopStreamSource[StreamNumber];
}


/*====================
  D3D_PushStreamSource
  ====================*/
bool	D3D_PushStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9 *pStreamData, UINT OffsetInBytes, UINT Stride)
{
	assert(StreamNumber >= 0 && StreamNumber < NUM_STREAMS);
	assert(g_StreamSource[StreamNumber].size() < 256);

	IDirect3DVertexBuffer9 *pCurrentTexture = g_StreamSource[StreamNumber].back();

	// Push the new state onto the stack
    g_StreamSource[StreamNumber].push_back(pStreamData);

	// Inform D3D API if the state actually changes
	if (pCurrentTexture != pStreamData)
	{
		g_pd3dDevice->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
		return true;
	}

	return false;
}


/*====================
  D3D_PushStreamSource
  ====================*/
bool	D3D_PushStreamSource(UINT StreamNumber)
{
	assert(StreamNumber >= 0 && StreamNumber < NUM_STREAMS);
	assert(g_StreamSource[StreamNumber].size() < 256);

	// Push a duplicate state onto the stack
    g_StreamSource[StreamNumber].push_back(g_StreamSource[StreamNumber].back());

	return false;
}


/*====================
  D3D_PopStreamSource
  ====================*/
bool	D3D_PopStreamSource(UINT StreamNumber)
{
	assert(false); // this function doesn't work yet!
	assert(StreamNumber >= 0 && StreamNumber < NUM_STREAMS);

	IDirect3DVertexBuffer9 *pOldStreamData = g_StreamSource[StreamNumber].back();

	// Pop the old state off the stack
    g_StreamSource[StreamNumber].pop_back();

	IDirect3DVertexBuffer9 *pNewStreamData = g_StreamSource[StreamNumber].back();

	// Inform D3D API if the state actually changes
	if (pOldStreamData != pNewStreamData)
	{
		g_pd3dDevice->SetStreamSource(StreamNumber, pNewStreamData, 0, 0/*TODO: This needs to be the real stride*/);
		return true;
	}

	return false;
}


/*====================
  D3D_SetVertexShaderDefault
  ====================*/
bool	D3D_SetVertexShaderDefault(IDirect3DVertexShader9 *pShader)
{
	// Clear the stack
	g_VertexShader.clear();

	// Push the default state as the bottom element of the stack
    g_TopVertexShader = pShader;

	// Actually set the texture
	g_pd3dDevice->SetVertexShader(pShader);
	return true;
}


/*====================
  D3D_GetVertexShader
  ====================*/
IDirect3DVertexShader9*	D3D_GetVertexShader()
{
	return g_TopVertexShader;
}


/*====================
  D3D_PushVertexShader
  ====================*/
bool	D3D_PushVertexShader(IDirect3DVertexShader9 *pShader)
{
	assert(g_VertexShader.size() < 256);

	IDirect3DVertexShader9 *pCurrentShader = g_VertexShader.back();

	// Push the new state onto the stack
    g_VertexShader.push_back(pShader);

	// Inform D3D API if the state actually changes
	if (pCurrentShader != pShader)
	{
		g_pd3dDevice->SetVertexShader(pShader);
		return true;
	}

	return false;
}


/*====================
  D3D_PushVertexShader
  ====================*/
bool	D3D_PushVertexShader()
{
	assert(g_VertexShader.size() < 256);

	// Push a duplicate state onto the stack
    g_VertexShader.push_back(g_VertexShader.back());

	return false;
}


/*====================
  D3D_PopVertexShader
  ====================*/
bool	D3D_PopVertexShader()
{
	IDirect3DVertexShader9 *pOldShader = g_VertexShader.back();

	// Pop the old state off the stack
    g_VertexShader.pop_back();

	IDirect3DVertexShader9 *pNewShader = g_VertexShader.back();

	// Inform D3D API if the state actually changes
	if (pOldShader != pNewShader)
	{
		g_pd3dDevice->SetVertexShader(pNewShader);
		return true;
	}

	return false;
}


/*====================
  D3D_SetPixelShaderDefault
  ====================*/
bool	D3D_SetPixelShaderDefault(IDirect3DPixelShader9 *pShader)
{
	// Clear the stack
	g_PixelShader.clear();

	// Push the default state as the bottom element of the stack
    g_TopPixelShader = pShader;

	// Actually set the texture
	g_pd3dDevice->SetPixelShader(pShader);
	return true;
}


/*====================
  D3D_GetPixelShader
  ====================*/
IDirect3DPixelShader9*	D3D_GetPixelShader()
{
	return g_TopPixelShader;
}


/*====================
  D3D_PushPixelShader
  ====================*/
bool	D3D_PushPixelShader(IDirect3DPixelShader9 *pShader)
{
	assert(g_PixelShader.size() < 256);

	IDirect3DPixelShader9 *pCurrentShader = g_PixelShader.back();

	// Push the new state onto the stack
    g_PixelShader.push_back(pShader);

	// Inform D3D API if the state actually changes
	if (pCurrentShader != pShader)
	{
		g_pd3dDevice->SetPixelShader(pShader);
		return true;
	}

	return false;
}


/*====================
  D3D_PushPixelShader
  ====================*/
bool	D3D_PushPixelShader()
{
	assert(g_PixelShader.size() < 256);

	// Push a duplicate state onto the stack
    g_PixelShader.push_back(g_PixelShader.back());

	return false;
}


/*====================
  D3D_PopPixelShader
  ====================*/
bool	D3D_PopPixelShader()
{
	IDirect3DPixelShader9 *pOldShader = g_PixelShader.back();

	// Pop the old state off the stack
    g_PixelShader.pop_back();

	IDirect3DPixelShader9 *pNewShader = g_PixelShader.back();

	// Inform D3D API if the state actually changes
	if (pOldShader != pNewShader)
	{
		g_pd3dDevice->SetPixelShader(pNewShader);
		return true;
	}

	return false;
}


/*====================
  D3D_SetVertexDeclarationDefault
  ====================*/
bool	D3D_SetVertexDeclarationDefault(IDirect3DVertexDeclaration9 *pDecl)
{
	// Clear the stack
	g_VertexDeclaration.clear();

	// Push the default state as the bottom element of the stack
    g_TopVertexDeclaration = pDecl;

	// Actually set the texture
	g_pd3dDevice->SetVertexDeclaration(pDecl);
	return true;
}


/*====================
  D3D_GetVertexDeclaration
  ====================*/
IDirect3DVertexDeclaration9*	D3D_GetVertexDeclaration()
{
	return g_TopVertexDeclaration;
}


/*====================
  D3D_PushVertexDeclaration
  ====================*/
bool	D3D_PushVertexDeclaration(IDirect3DVertexDeclaration9 *pDecl)
{
	assert(g_VertexDeclaration.size() < 256);

	IDirect3DVertexDeclaration9 *pCurrentDecl = g_VertexDeclaration.back();

	// Push the new state onto the stack
    g_VertexDeclaration.push_back(pDecl);

	// Inform D3D API if the state actually changes
	if (pCurrentDecl != pDecl)
	{
		g_pd3dDevice->SetVertexDeclaration(pDecl);
		return true;
	}

	return false;
}


/*====================
  D3D_PushVertexDeclaration
  ====================*/
bool	D3D_PushVertexDeclaration()
{
	assert(g_VertexDeclaration.size() < 256);

	// Push a duplicate state onto the stack
    g_VertexDeclaration.push_back(g_VertexDeclaration.back());

	return false;
}


/*====================
  D3D_PopVertexDeclaration
  ====================*/
bool	D3D_PopVertexDeclaration()
{
	IDirect3DVertexDeclaration9 *pOldDecl = g_VertexDeclaration.back();

	// Pop the old state off the stack
    g_VertexDeclaration.pop_back();

	IDirect3DVertexDeclaration9 *pNewDecl = g_VertexDeclaration.back();

	// Inform D3D API if the state actually changes
	if (pOldDecl != pNewDecl)
	{
		g_pd3dDevice->SetVertexDeclaration(pNewDecl);
		return true;
	}

	return false;
}
