#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "VoxelWorld.generated.h"

UCLASS()
class TERRAVOXUE5_API AVoxelWorld : public AActor
{
    GENERATED_BODY()

public:
    AVoxelWorld();
    virtual void BeginPlay() override;

    // ---------- Public API (used by GameMode / PlayerPawn) ----------
    int32 GetBlock(int32 X, int32 Y, int32 Z) const;
    int32 GetTerrainHeight(int32 X, int32 Z) const;
    void SetBlock(int32 X, int32 Y, int32 Z, int32 ID);
    void RebuildChunksInRadius(int32 X, int32 Z, int32 Radius);

    UPROPERTY(VisibleAnywhere) UProceduralMeshComponent* VoxelMesh;

    UPROPERTY(EditAnywhere, Category = "TerraVox") int32 RenderDistance = 4;
    UPROPERTY(EditAnywhere, Category = "TerraVox") int32 TerrainSeed = 1337;
    UPROPERTY(EditAnywhere, Category = "TerraVox") float HeightScale = 22.f;

    static constexpr int32 ChunkSize = 16;
    static constexpr int32 TerrainDepth = 64;

private:
    // chunk key (cx, 0, cz) -> block ids
    TMap<FIntVector, TArray<int8>> Blocks;
    // chunk key -> PMC section index
    TMap<FIntVector, int32> ChunkSections;
    // chunk key -> (verts, tris) for diagnostics
    TMap<FIntVector, FIntPoint> ChunkStats;
    int32 SectionCounter = 0;

    int32 TerrainHeightAt(int32 X, int32 Z) const;
    float Noise2D(float X, float Z) const;

    void GenerateWorld();
    void FillChunk(int32 CX, int32 CZ);
    void WriteBlockRaw(int32 X, int32 Y, int32 Z, int32 ID);
    void MeshChunk(int32 CX, int32 CZ);

    static FIntVector ChunkKey(int32 CX, int32 CZ) { return FIntVector(CX, 0, CZ); }
    static int32 FlatToChunk(int32 V) { return FMath::FloorToInt((float)V / ChunkSize); }

    static FColor BlockColor(int32 ID);
    static float FaceShade(const FVector& Normal);
    static int32 BlockGrain(int32 X, int32 Y, int32 Z);

    void AddFace(TArray<FVector>& Verts, TArray<int32>& Tris, TArray<FVector>& Normals,
                 TArray<FVector2D>& UVs, TArray<FColor>& Colors, TArray<FProcMeshTangent>& Tangents,
                 const FVector& V0, const FVector& V1, const FVector& V2, const FVector& V3,
                 const FVector& Normal, const FColor& Color);
};
