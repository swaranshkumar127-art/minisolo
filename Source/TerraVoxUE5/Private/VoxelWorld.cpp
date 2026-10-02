#include "VoxelWorld.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"

AVoxelWorld::AVoxelWorld()
{
    // Single shared PMC for the whole world. Created in the CDO so it is
    // guaranteed registered & rendered. Unlit engine vertex-colour material
    // chosen because runtime-created lit materials async-compile forever on
    // this weak iGPU (hard-won lesson #3/#4).
    VoxelMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("VoxelMesh"));
    SetRootComponent(VoxelMesh);

    static ConstructorHelpers::FObjectFinder<UMaterial> MatAsset(TEXT("/Engine/EngineDebugMaterials/VertexColorMaterial"));
    if (MatAsset.Succeeded())
    {
        VoxelMesh->SetMaterial(0, MatAsset.Object);
    }

    PrimaryActorTick.bCanEverTick = false;
}

void AVoxelWorld::BeginPlay()
{
    Super::BeginPlay();
    GenerateWorld();
}

// -------------------- generation --------------------

float AVoxelWorld::Noise2D(float X, float Z) const
{
    auto Hash = [](int32 A, int32 B) -> float
    {
        int32 H = A * 374761393 + B * 668265263;
        H = (H ^ (H >> 13)) * 1274126177;
        H = H ^ (H >> 16);
        return ((float)(H & 0xFFFF)) / 32768.f - 1.f;
    };
    auto Fade = [](float T) { return T * T * (3.f - 2.f * T); };

    const float SX = X + (float)TerrainSeed;
    const float SZ = Z + (float)TerrainSeed;

    int32 X0 = FMath::FloorToInt(SX);
    int32 Z0 = FMath::FloorToInt(SZ);
    float Xf = SX - (float)X0;
    float Zf = SZ - (float)Z0;

    float V00 = Hash(X0, Z0);
    float V10 = Hash(X0 + 1, Z0);
    float V01 = Hash(X0, Z0 + 1);
    float V11 = Hash(X0 + 1, Z0 + 1);

    float U = Fade(Xf);
    float V = Fade(Zf);

    return FMath::Lerp(FMath::Lerp(V00, V10, U), FMath::Lerp(V01, V11, U), V);
}

int32 AVoxelWorld::TerrainHeightAt(int32 X, int32 Z) const
{
    // H = 24 + HeightScale * (N + 0.6 * N2), clamped to [1, 63]
    float N = Noise2D((float)X * 0.03f, (float)Z * 0.03f);
    float N2 = Noise2D((float)X * 0.008f + 31.7f, (float)Z * 0.008f + 11.3f);
    float H = 24.f + HeightScale * (N + 0.6f * N2);
    return FMath::Clamp((int32)H, 1, TerrainDepth - 2);
}

void AVoxelWorld::WriteBlockRaw(int32 X, int32 Y, int32 Z, int32 ID)
{
    if (Y < 0 || Y >= TerrainDepth)
    {
        return;
    }
    const int32 CX = FlatToChunk(X);
    const int32 CZ = FlatToChunk(Z);
    TArray<int8>& Arr = Blocks.FindOrAdd(ChunkKey(CX, CZ));
    if (Arr.Num() == 0)
    {
        Arr.Init(0, ChunkSize * ChunkSize * TerrainDepth);
    }
    const int32 LX = X - CX * ChunkSize;
    const int32 LZ = Z - CZ * ChunkSize;
    const int32 Idx = LX + ChunkSize * (LZ + ChunkSize * Y);
    Arr[Idx] = (int8)ID;
}

void AVoxelWorld::FillChunk(int32 CX, int32 CZ)
{
    const int32 BaseX = CX * ChunkSize;
    const int32 BaseZ = CZ * ChunkSize;

    for (int32 LX = 0; LX < ChunkSize; ++LX)
    {
        for (int32 LZ = 0; LZ < ChunkSize; ++LZ)
        {
            const int32 X = BaseX + LX;
            const int32 Z = BaseZ + LZ;
            const int32 H = TerrainHeightAt(X, Z);

            WriteBlockRaw(X, 0, Z, 3); // bedrock at y=0 (stone palette)
            for (int32 Y = 1; Y <= H; ++Y)
            {
                WriteBlockRaw(X, Y, Z, 3); // stone below
            }
        }
    }
}

void AVoxelWorld::GenerateWorld()
{
    // Phase 1: raw columns for every chunk in range
    for (int32 CX = -RenderDistance; CX <= RenderDistance; ++CX)
    {
        for (int32 CZ = -RenderDistance; CZ <= RenderDistance; ++CZ)
        {
            FillChunk(CX, CZ);
        }
    }

    // Phase 2: grass top + pines (second pass so crown blocks can cross chunk borders)
    const int32 Start = -RenderDistance * ChunkSize;
    const int32 End = RenderDistance * ChunkSize + ChunkSize - 1;
    for (int32 X = Start; X <= End; ++X)
    {
        for (int32 Z = Start; Z <= End; ++Z)
        {
            int32 H = 0;
            for (int32 Y = TerrainDepth - 1; Y >= 0; --Y)
            {
                if (GetBlock(X, Y, Z) != 0) { H = Y; break; }
            }
            WriteBlockRaw(X, H, Z, 1); // grass on top
            if (H - 1 >= 0)
            {
                WriteBlockRaw(X, H - 1, Z, 2); // dirt layer under grass
            }

            // taiga pines: deterministic ~10% chance when H in 20..52
            int32 TreeChance = FMath::Abs(FMath::HashCombine(FMath::HashCombine(X, Z), TerrainSeed)) % 100;
            if (TreeChance < 10 && H >= 20 && H <= 52)
            {
                const int32 TrunkH = 4 + (FMath::Abs(FMath::HashCombine(X, Z * 7)) % 3); // 4-6
                for (int32 Y = H + 1; Y <= H + TrunkH; ++Y)
                {
                    WriteBlockRaw(X, Y, Z, 5); // log
                }
                const int32 TopY = H + TrunkH;
                for (int32 L = 0; L < 3; ++L)
                {
                    const int32 Y = TopY - 1 - L;
                    const int32 R = (L == 0) ? 1 : (L == 1 ? 1 : 0);
                    for (int32 DX = -R; DX <= R; ++DX)
                    {
                        for (int32 DZ = -R; DZ <= R; ++DZ)
                        {
                            if (DX == 0 && DZ == 0 && L > 0) { continue; } // keep trunk
                            WriteBlockRaw(X + DX, Y, Z + DZ, 6); // leaves
                        }
                    }
                }
                WriteBlockRaw(X, TopY + 1, Z, 6); // crown tip
            }
        }
    }

    // Phase 3: mesh every chunk
    for (int32 CX = -RenderDistance; CX <= RenderDistance; ++CX)
    {
        for (int32 CZ = -RenderDistance; CZ <= RenderDistance; ++CZ)
        {
            MeshChunk(CX, CZ);
        }
    }

    const FIntPoint* Stats = ChunkStats.Find(ChunkKey(0, 0));
    UE_LOG(LogTemp, Log, TEXT("VW: chunk[0,0] section=%d verts=%d tris=%d registered=1 sections=%d"),
        ChunkSections.FindRef(ChunkKey(0, 0)), Stats ? Stats->X : 0, Stats ? Stats->Y : 0, ChunkSections.Num());
}

// -------------------- queries --------------------

int32 AVoxelWorld::GetBlock(int32 X, int32 Y, int32 Z) const
{
    if (Y < 0 || Y >= TerrainDepth)
    {
        return 0;
    }
    const FIntVector Key = ChunkKey(FlatToChunk(X), FlatToChunk(Z));
    const TArray<int8>* Arr = Blocks.Find(Key);
    if (!Arr || Arr->Num() == 0)
    {
        return 0;
    }
    const int32 CX = Key.X * ChunkSize;
    const int32 CZ = Key.Z * ChunkSize;
    const int32 LX = X - CX;
    const int32 LZ = Z - CZ;
    return (*Arr)[LX + ChunkSize * (LZ + ChunkSize * Y)];
}

int32 AVoxelWorld::GetTerrainHeight(int32 X, int32 Z) const
{
    for (int32 Y = TerrainDepth - 1; Y >= 0; --Y)
    {
        if (GetBlock(X, Y, Z) != 0)
        {
            return Y;
        }
    }
    return 0;
}

void AVoxelWorld::SetBlock(int32 X, int32 Y, int32 Z, int32 ID)
{
    WriteBlockRaw(X, Y, Z, ID);
    RebuildChunksInRadius(X, Z, 1);
}

void AVoxelWorld::RebuildChunksInRadius(int32 X, int32 Z, int32 Radius)
{
    const int32 CX = FlatToChunk(X);
    const int32 CZ = FlatToChunk(Z);
    for (int32 DX = -Radius; DX <= Radius; ++DX)
    {
        for (int32 DZ = -Radius; DZ <= Radius; ++DZ)
        {
            MeshChunk(CX + DX, CZ + DZ);
        }
    }
}

// -------------------- meshing --------------------

FColor AVoxelWorld::BlockColor(int32 ID)
{
    switch (ID)
    {
    case 1: return FColor(104, 158, 64);   // grass
    case 2: return FColor(132, 94, 66);    // dirt
    case 3: return FColor(124, 124, 130);  // stone
    case 4: return FColor(164, 132, 74);   // plank
    case 5: return FColor(101, 73, 51);    // pine log
    case 6: return FColor(52, 112, 40);    // pine leaves
    default: return FColor::White;
    }
}

float AVoxelWorld::FaceShade(const FVector& Normal)
{
    if (Normal.Y > 0.5f) return 1.0f;   // top
    if (Normal.Y < -0.5f) return 0.68f; // down
    if (Normal.X != 0.f) return 0.82f;
    return 0.9f;
}

int32 AVoxelWorld::BlockGrain(int32 X, int32 Y, int32 Z)
{
    uint32 H = (uint32)(X * 73856093) ^ (uint32)(Z * 19349663) ^ (uint32)(Y * 83492791);
    return (int32)(H & 7) - 3;
}

void AVoxelWorld::AddFace(TArray<FVector>& Verts, TArray<int32>& Tris, TArray<FVector>& Normals,
                          TArray<FVector2D>& UVs, TArray<FColor>& Colors, TArray<FProcMeshTangent>& Tangents,
                          const FVector& V0, const FVector& V1, const FVector& V2, const FVector& V3,
                          const FVector& Normal, const FColor& Color)
{
    const int32 Base = Verts.Num();
    Verts.Add(V0); Verts.Add(V1); Verts.Add(V2); Verts.Add(V3);
    // NOTE: winding chosen so fronts face outward in the verified original build.
    // If the terrain ever renders inside-out, swap these two index triples' order.
    Tris.Add(Base + 0); Tris.Add(Base + 2); Tris.Add(Base + 1);
    Tris.Add(Base + 0); Tris.Add(Base + 3); Tris.Add(Base + 2);
    for (int32 i = 0; i < 4; ++i)
    {
        Normals.Add(Normal);
        UVs.Add(FVector2D::ZeroVector);
        Colors.Add(Color);
        Tangents.Add(FProcMeshTangent());
    }
}

void AVoxelWorld::MeshChunk(int32 CX, int32 CZ)
{
    const FIntVector Key = ChunkKey(CX, CZ);
    TArray<int8>* Arr = Blocks.Find(Key);
    if (!Arr)
    {
        return;
    }

    TArray<FVector> Verts;
    TArray<int32> Tris;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FColor> Colors;
    TArray<FProcMeshTangent> Tangents;

    const int32 BaseX = CX * ChunkSize;
    const int32 BaseZ = CZ * ChunkSize;

    for (int32 LX = 0; LX < ChunkSize; ++LX)
    {
        for (int32 LZ = 0; LZ < ChunkSize; ++LZ)
        {
            for (int32 Y = 0; Y < TerrainDepth; ++Y)
            {
                const int32 X = BaseX + LX;
                const int32 Z = BaseZ + LZ;
                const int32 ID = GetBlock(X, Y, Z);
                if (ID == 0)
                {
                    continue;
                }

                const FColor Base = BlockColor(ID);
                const int32 Grain = BlockGrain(X, Y, Z);
                const FVector P((float)X, (float)Y, (float)Z);

                // 6 faces, culled against non-air neighbours
                if (GetBlock(X + 1, Y, Z) == 0)
                {
                    float S = FaceShade(FVector(1, 0, 0));
                    FColor C = FColor(FMath::Clamp(Base.R + Grain, 0, 255), FMath::Clamp(Base.G + Grain, 0, 255), FMath::Clamp(Base.B + Grain, 0, 255));
                    C.R = (uint8)(C.R * S); C.G = (uint8)(C.G * S); C.B = (uint8)(C.B * S);
                    AddFace(Verts, Tris, Normals, UVs, Colors, Tangents,
                        P + FVector(1, 0, 0), P + FVector(1, 1, 0), P + FVector(1, 1, 1), P + FVector(1, 0, 1),
                        FVector(1, 0, 0), C);
                }
                if (GetBlock(X - 1, Y, Z) == 0)
                {
                    float S = FaceShade(FVector(-1, 0, 0));
                    FColor C = Base; C.R = (uint8)(C.R * S); C.G = (uint8)(C.G * S); C.B = (uint8)(C.B * S);
                    AddFace(Verts, Tris, Normals, UVs, Colors, Tangents,
                        P + FVector(0, 0, 1), P + FVector(0, 1, 1), P + FVector(0, 1, 0), P + FVector(0, 0, 0),
                        FVector(-1, 0, 0), C);
                }
                if (GetBlock(X, Y + 1, Z) == 0)
                {
                    float S = FaceShade(FVector(0, 1, 0));
                    FColor C = Base; C.R = (uint8)(C.R * S); C.G = (uint8)(C.G * S); C.B = (uint8)(C.B * S);
                    AddFace(Verts, Tris, Normals, UVs, Colors, Tangents,
                        P + FVector(0, 1, 0), P + FVector(0, 1, 1), P + FVector(1, 1, 1), P + FVector(1, 1, 0),
                        FVector(0, 1, 0), C);
                }
                if (GetBlock(X, Y - 1, Z) == 0)
                {
                    float S = FaceShade(FVector(0, -1, 0));
                    FColor C = Base; C.R = (uint8)(C.R * S); C.G = (uint8)(C.G * S); C.B = (uint8)(C.B * S);
                    AddFace(Verts, Tris, Normals, UVs, Colors, Tangents,
                        P + FVector(0, 0, 1), P + FVector(1, 0, 1), P + FVector(1, 0, 0), P + FVector(0, 0, 0),
                        FVector(0, -1, 0), C);
                }
                if (GetBlock(X, Y, Z + 1) == 0)
                {
                    float S = FaceShade(FVector(0, 0, 1));
                    FColor C = Base; C.R = (uint8)(C.R * S); C.G = (uint8)(C.G * S); C.B = (uint8)(C.B * S);
                    AddFace(Verts, Tris, Normals, UVs, Colors, Tangents,
                        P + FVector(0, 0, 1), P + FVector(1, 0, 1), P + FVector(1, 1, 1), P + FVector(0, 1, 1),
                        FVector(0, 0, 1), C);
                }
                if (GetBlock(X, Y, Z - 1) == 0)
                {
                    float S = FaceShade(FVector(0, 0, -1));
                    FColor C = Base; C.R = (uint8)(C.R * S); C.G = (uint8)(C.G * S); C.B = (uint8)(C.B * S);
                    AddFace(Verts, Tris, Normals, UVs, Colors, Tangents,
                        P + FVector(1, 0, 0), P + FVector(0, 0, 0), P + FVector(0, 1, 0), P + FVector(1, 1, 0),
                        FVector(0, 0, -1), C);
                }
            }
        }
    }

    int32* SectionIdx = ChunkSections.Find(Key);
    int32 Section = 0;
    if (SectionIdx)
    {
        Section = *SectionIdx;
        VoxelMesh->ClearMeshSection(Section);
    }
    else
    {
        Section = SectionCounter++;
        ChunkSections.Add(Key, Section);
    }

    VoxelMesh->CreateMeshSection(Section, Verts, Tris, Normals, UVs, Colors, Tangents, true);
    ChunkStats.Add(Key, FIntPoint(Verts.Num(), Tris.Num()));
}
