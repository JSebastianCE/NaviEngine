#pragma once

/**
 * @file Prerequisites.h
 * @brief Archivo de cabecera principal que incluye librerías estándar, DirectX, dependencias de terceros y define estructuras y macros globales.
 */

 // Librerias STD
#include <string>
#include <sstream>
#include <vector>
#include <windows.h>
#include <xnamath.h>
#include <thread>
#include <array>

#include <memory>
#include <unordered_map>
#include <type_traits>

// Librerias DirectX
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include "Resource.h"
#include "resource.h"

// Third Party Libraries
#include "EngineUtilities/Vectors/Vector2.h"
#include "EngineUtilities/Vectors/Vector3.h"
#include "EngineUtilities\Memory\TSharedPointer.h"
#include "EngineUtilities\Memory\TWeakPointer.h"
#include "EngineUtilities\Memory\TStaticPtr.h"
#include "EngineUtilities\Memory\TUniquePtr.h"

// MACROS

/**
 * @brief Libera de manera segura un recurso de DirectX.
 *
 * Si el puntero no es nulo, libera la memoria con Release()
 * y lo asigna a nullptr para evitar accesos inválidos.
 *
 * @param x Puntero al recurso que se va a liberar.
 */
#define SAFE_RELEASE(x) if(x != nullptr) x->Release(); x = nullptr;

 /*
  * @brief Macro para mostrar mensajes de creación de recursos en la ventana de depuración.
  *
  * Formatea un mensaje con la clase, el método y el estado actual de la creación.
  *
  * @param classObj Nombre de la clase donde ocurre el evento.
  * @param method Nombre del método donde ocurre el evento.
  * @param state Estado del recurso (ejemplo: "OK", "FAILED").
  */

  /*
  #define MESSAGE( classObj, method, state )   \
  {                                            \
     std::wostringstream os_;                  \
     os_ << classObj << "::" << method << " : " << "[CREATION OF RESOURCE " << ": " << state << "] \n"; \
     OutputDebugStringW( os_.str().c_str() );  \
  }*/

  /*
   * @brief Macro para registrar mensajes de error en la ventana de depuración.
   *
   * Captura información detallada de la clase, método y descripción del error.
   * Si ocurre un fallo durante el registro, captura la excepción y notifica.
   *
   * @param classObj Nombre de la clase donde ocurre el error.
   * @param method Nombre del método donde ocurre el error.
   * @param errorMSG Mensaje descriptivo del error.
   */

   /*
#define ERROR(classObj, method, errorMSG)                     \
{                                                             \
    try {                                                     \
        std::wostringstream os_;                              \
        os_ << L"ERROR : " << classObj << L"::" << method     \
            << L" : " << errorMSG << L"\n";                   \
        OutputDebugStringW(os_.str().c_str());                \
    } catch (...) {                                           \
        OutputDebugStringW(L"Failed to log error message.\n");\
    }                                                         \
}*/



// SISTEMA DE LOGS PARA EL EDITOR ---

/**
 * @struct LogEntry
 * @brief Representa una entrada individual en el sistema de registro (log) del editor.
 */
struct LogEntry {
  std::string text; /**< Texto descriptivo del mensaje. */
  int type;         /**< Tipo de registro: 0 = Info/Message, 1 = Warning, 2 = Error. */
};

/**
 * @class EditorLog
 * @brief Clase estática que gestiona y almacena los registros de depuración para la interfaz del editor.
 */
class EditorLog {
public:
  inline static std::vector<LogEntry> s_Logs; /**< Almacenamiento global de las entradas del registro. */

  /**
   * @brief Añade un nuevo mensaje al sistema de registro y a la salida de depuración de Visual Studio.
   *
   * @param wtext Texto del mensaje en formato de cadena amplia (std::wstring).
   * @param type Tipo de mensaje (0 = Info, 1 = Warning, 2 = Error).
   */
  static void AddLog(const std::wstring& wtext, int type) {
    // Enviar a Visual Studio como antes
    OutputDebugStringW(wtext.c_str());

    // Convertir Wide String (wstring) a String normal (UTF-8) para ImGui
    std::string text(wtext.begin(), wtext.end());
    s_Logs.push_back({ text, type });
  }

  /**
   * @brief Limpia todos los mensajes almacenados en el registro del editor.
   */
  static void Clear() {
    s_Logs.clear();
  }
};

/**
 * @def MESSAGE
 * @brief Macro activa para registrar mensajes de éxito/estado directamente en el EditorLog.
 * @param classObj Nombre de la clase.
 * @param method Nombre del método.
 * @param state Estado de la operación.
 */
#define MESSAGE( classObj, method, state )   \
{                                            \
   std::wostringstream os_;                  \
   os_ << classObj << "::" << method << " : [CREATION OF RESOURCE : " << state << "] \n"; \
   EditorLog::AddLog(os_.str(), 0);          \
}

 /**
  * @def ERROR
  * @brief Macro activa para registrar mensajes de error directamente en el EditorLog.
  * @param classObj Nombre de la clase.
  * @param method Nombre del método.
  * @param errorMSG Mensaje de error a registrar.
  */
#define ERROR(classObj, method, errorMSG)                     \
{                                                             \
    try {                                                     \
        std::wostringstream os_;                              \
        os_ << L"ERROR : " << classObj << L"::" << method     \
            << L" : " << errorMSG << L"\n";                   \
        EditorLog::AddLog(os_.str(), 2);                      \
    } catch (...) {                                           \
        OutputDebugStringW(L"Failed to log error message.\n");\
    }                                                         \
}

  /**
   * @struct SimpleVertex
   * @brief Representa un vértice estándar utilizado para la geometría 3D, incluyendo datos de iluminación y texturizado.
   */
struct
  SimpleVertex {
  EU::Vector3 Position;          /**< Coordenadas de posición del vértice (x, y, z). */
  EU::Vector3 Normal;            /**< Vector normal del vértice (para iluminación). */
  EU::Vector3 Tangent;           /**< Vector tangente del vértice (para normal mapping e iluminación). */
  EU::Vector3 Bitangent;         /**< Vector bitangente del vértice (para normal mapping e iluminación). */
  EU::Vector2 TextureCoordinate; /**< Coordenadas de textura (u, v). */
};

/**
 * @struct SkyboxVertex
 * @brief Representa un vértice básico utilizado exclusivamente para dibujar el cubo del Skybox.
 */
struct
  SkyboxVertex {
  float x, y, z; /**< Coordenadas espaciales del vértice. */
};

/**
 * @struct CBSkybox
 * @brief Buffer constante utilizado para pasar la matriz de transformación al shader del Skybox.
 */
struct CBSkybox
{
  XMMATRIX mviewProj; /**< Matriz combinada de Vista y Proyección. */
};

/**
 * @struct LoadData
 * @brief Estructura de utilidad para almacenar temporalmente los datos cargados de una malla (Mesh).
 */
struct
  LoadData {
  std::string name;                   /**< Nombre o identificador del modelo. */
  std::vector <SimpleVertex> vertex;  /**< Lista de vértices del modelo. */
  std::vector <unsigned int> index;   /**< Lista de índices para el renderizado. */
  int numVertex;                      /**< Cantidad total de vértices. */
  int numIndex;                       /**< Cantidad total de índices. */
};

/**
 * @struct CBNeverChanges
 * @brief Constantes de shader que nunca cambian durante la ejecución: contiene la matriz de vista.
 */
struct
  CBNeverChanges {
  XMMATRIX mView; /**< Matriz de vista usada en la cámara. */
};

/**
 * @struct CBChangeOnResize
 * @brief Constantes de shader que cambian únicamente al redimensionar la ventana de la aplicación.
 */
struct
  CBChangeOnResize {
  XMMATRIX mProjection; /**< Matriz de proyección ajustada a la relación de aspecto de la ventana. */
};

/**
 * @struct CBMain
 * @brief Buffer constante principal de la escena, agrupa matrices fundamentales y datos globales de iluminación.
 */
struct CBMain
{
  //XMOFLOAT4X4 World;
  XMFLOAT4X4 View;           /**< Matriz de vista actual. */
  XMFLOAT4X4 Projection;     /**< Matriz de proyección actual. */
  EU::Vector3 CameraPos;     /**< Posición de la cámara en el espacio del mundo. */
  float pad0;                /**< Padding para alineación de memoria (HLSL requiere bloques de 16 bytes). */
  EU::Vector3 LightDir;      /**< Dirección de la luz principal. */
  float pad1;                /**< Padding para alineación de memoria. */
  EU::Vector3 LightColor;    /**< Color e intensidad de la luz principal. */
  float pad2;                /**< Padding para alineación de memoria. */
};


/**
 * @struct CBChangesEveryFrame
 * @brief Constantes de shader que cambian en cada frame renderizado por objeto.
 */
struct
  CBChangesEveryFrame {
  XMMATRIX mWorld;      /**< Matriz de mundo (World) para transformar los objetos individualmente. */
  XMFLOAT4 vMeshColor;  /**< Color base aplicado a la malla. */
};

/**
 * @enum ExtensionType
 * @brief Define los tipos de extensión soportados para las texturas.
 */
enum
  ExtensionType {
  DDS = 0, /**< Textura en formato DDS (DirectDraw Surface). */
  PNG = 1, /**< Textura en formato PNG (Portable Network Graphics). */
  JPG = 2  /**< Textura en formato JPG (Joint Photographic Experts Group). */
};

/**
 * @enum ShaderType
 * @brief Identificadores básicos para los distintos tipos de Shaders en el pipeline.
 */
enum
  ShaderType {
  VERTEX_SHADER = 0, /**< Representa un Vertex Shader. */
  PIXEL_SHADER = 1   /**< Representa un Pixel Shader. */
};

/**
 * @enum ComponentType
 * @brief Enumerador para identificar el tipo subyacente de cada Componente en la arquitectura ECS.
 */
enum
  ComponentType {
  NONE = 0,             /**< Componente nulo o no asignado. */
  TRANSFORM = 1,        /**< Componente de transformación espacial (Posición, Rotación, Escala). */
  MESH = 2,             /**< Componente de malla 3D. */
  MATERIAL = 3,         /**< Componente de material visual. */
  HIERARCHY = 4,        /**< Componente para jerarquías padre/hijo. */
  PARTICLE_EMITTER = 5  /**< Componente generador de sistemas de partículas. */
};

/**
 * @struct ParticleVertex
 * @brief Vértice específico para el renderizado de partículas, optimizado para Billboarding y SDF.
 */
struct 
ParticleVertex {
  XMFLOAT3 Position;   /**< Coordenada central de la partícula en el mundo (alineado con DirectX). */
  XMFLOAT4 Color;      /**< Color actual de la partícula, incluyendo el canal Alfa. */
  float    Size;       /**< Escala o tamaño de la partícula. */
  unsigned int ShapeType; /**< Identificador numérico de la forma geométrica (SDF) a dibujar. */
};

/**
 * @struct CBParticleMatrices
 * @brief Buffer constante para el Vertex Shader de partículas.
 *
 * Se utiliza __declspec(align(16)) para forzar una alineación estricta de 16 bytes en la memoria,
 * lo cual es un requisito arquitectónico inquebrantable de los Constant Buffers en DirectX 11.
 */
__declspec(align(16)) struct CBParticleMatrices {
  XMMATRIX mWorld;       /**< Matriz de mundo para el emisor de partículas. */
  XMMATRIX mView;        /**< Matriz de vista de la cámara. */
  XMMATRIX mProjection;  /**< Matriz de proyección. */
  XMFLOAT4 OutlineColor; /**< Color de contorno global para las partículas SDF. */
};