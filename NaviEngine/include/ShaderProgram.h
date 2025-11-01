#pragma once
#include "Prerequisites.h"
#include "InputLayout.h"

/**
 * @brief Declaraci�n adelantada de la clase Device.
 */
class
Device;

/**
 * @brief Declaraci�n adelantada de la clase DeviceContext.
 */
class
DeviceContext;

/**
 * @class ShaderProgram
 * @brief Clase encargada de manejar la inicializaci�n, compilaci�n,
 *        creaci�n y renderizado de shaders dentro del motor gr�fico.
 */
class
ShaderProgram {
public:

  /**
   * @brief Constructor por defecto de ShaderProgram.
   */
  ShaderProgram() = default;

  /**
   * @brief Destructor por defecto de ShaderProgram.
   */
  ~ShaderProgram() = default;

  /**
   * @brief Inicializa el programa de shaders cargando los archivos necesarios y configurando el layout.
   * @param device Referencia al dispositivo de renderizado.
   * @param fileName Nombre del archivo del shader.
   * @param Layout Vector con la descripci�n de los elementos de entrada del shader.
   * @return HRESULT que indica el resultado de la operaci�n.
   */
  HRESULT
  init(Device& device,
      const std::string& fileName,
      std::vector < D3D11_INPUT_ELEMENT_DESC> Layout);

  /**
   * @brief Actualiza los par�metros o recursos del shader si es necesario.
   */
  void
  update();

  /**
   * @brief Ejecuta el renderizado usando los shaders actuales.
   * @param deviceContext Contexto del dispositivo utilizado para dibujar.
   */
  void
  render(DeviceContext& deviceContext);

  /**
   * @brief Renderiza utilizando un tipo de shader espec�fico (Vertex, Pixel, etc.).
   * @param deviceContext Contexto del dispositivo utilizado para dibujar.
   * @param type Tipo de shader a usar.
   */
  void
  render(DeviceContext& deviceContext, ShaderType type);

  /**
   * @brief Libera los recursos asociados a los shaders.
   */
  void
  destroy();

  /**
   * @brief Crea el Input Layout del shader para definir la estructura de los v�rtices.
   * @param device Referencia al dispositivo de renderizado.
   * @param Layout Vector con la descripci�n de los elementos de entrada.
   * @return HRESULT que indica el resultado de la creaci�n.
   */
  HRESULT
  CreateInputLayout(Device& device,
                    std::vector<D3D11_INPUT_ELEMENT_DESC> Layout);

  /**
   * @brief Crea un shader del tipo especificado.
   * @param device Referencia al dispositivo de renderizado.
   * @param type Tipo de shader a crear.
   * @return HRESULT que indica el resultado de la operaci�n.
   */
  HRESULT
  CreateShader(Device& device, ShaderType type);

  /**
   * @brief Crea un shader del tipo especificado desde un archivo.
   * @param device Referencia al dispositivo de renderizado.
   * @param type Tipo de shader a crear.
   * @param fileName Nombre del archivo del shader.
   * @return HRESULT que indica el resultado de la operaci�n.
   */
  HRESULT
  CreateShader(Device& device, ShaderType type, const std::string& fileName);

  /**
   * @brief Compila un shader desde un archivo fuente.
   * @param szFileName Nombre del archivo fuente del shader.
   * @param szEntryPoint Punto de entrada del shader.
   * @param szShaderModel Modelo de shader (por ejemplo, "vs_5_0", "ps_5_0").
   * @param ppBlobOut Puntero de salida donde se guarda el blob compilado.
   * @return HRESULT que indica el resultado de la compilaci�n.
   */
  HRESULT
  CompileShaderFromFile(char* szFileName,
                        LPCSTR szEntryPoint,
                        LPCSTR szShaderModel,
                        ID3DBlob** ppBlobOut);

public:
  /** @brief Puntero al Vertex Shader actual. */
  ID3D11VertexShader* m_VertexShader = nullptr;

  /** @brief Puntero al Pixel Shader actual. */
  ID3D11PixelShader* m_PixelShader = nullptr;

  /** @brief Layout de entrada usado para definir la estructura de los v�rtices. */
  InputLayout m_inputLayout;

private:

  /** @brief Nombre del archivo del shader cargado. */
  std::string m_shaderFileName;

  /** @brief Datos compilados del Vertex Shader. */
  ID3DBlob* m_vertexShaderData = nullptr;

  /** @brief Datos compilados del Pixel Shader. */
  ID3DBlob* m_pixelShaderData = nullptr;

};
