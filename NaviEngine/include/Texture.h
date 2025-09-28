#pragma once
#include "Prerequisites.h"

class 
Device;

class 
DeviceContext;

/**
 * @class Texture
 * @brief Representa una textura en DirectX 11.
 *
 * Esta clase encapsula la creación, gestión y destrucción de texturas 2D
 * en DirectX, así como su vinculación al pipeline gráfico. Puede inicializarse
 * desde archivo, como un recurso en memoria, o copiando otra textura.
 */
class
Texture {
public:
  /**
   * @brief Constructor por defecto.
   */
  Texture() = default;

  /**
   * @brief Destructor por defecto.
   */
  ~Texture() = default;

  /**
   * @brief Inicializa la textura desde un archivo de imagen.
   *
   * @param device Referencia al dispositivo de DirectX.
   * @param textureName Nombre o ruta del archivo de la textura.
   * @param extensionType Tipo de extensión de la textura (ej. PNG, JPG).
   * @return HRESULT Código de resultado (S_OK si se cargó correctamente).
   */
  HRESULT
  init(Device& device,
      const std::string& textureName,
      ExtensionType extensionType);

  /**
   * @brief Inicializa la textura como un recurso vacío en memoria.
   *
   * @param device Referencia al dispositivo de DirectX.
   * @param widht Ancho de la textura.
   * @param height Alto de la textura.
   * @param Format Formato de la textura (DXGI_FORMAT).
   * @param BindFlags Banderas de enlace (ej. render target, shader resource).
   * @param sampleCount Número de muestras para multisampling (default = 1).
   * @param qualityLevels Niveles de calidad para multisampling (default = 0).
   * @return HRESULT Código de resultado (S_OK si se creó correctamente).
   */
  HRESULT
  init(Device& device,
      unsigned int widht,
      unsigned int height,
      DXGI_FORMAT Format,
      unsigned int BindFlags,
      unsigned int sampleCount = 1,
      unsigned int qualityLevels = 0);

  /**
   * @brief Inicializa la textura copiando desde otra textura existente.
   *
   * @param device Referencia al dispositivo de DirectX.
   * @param textureRef Textura de referencia para crear la nueva.
   * @param format Formato de la textura (DXGI_FORMAT).
   * @return HRESULT Código de resultado.
   */
  HRESULT
  init(Device& device, Texture& textureRef, DXGI_FORMAT format);

  /**
   * @brief Actualiza el estado de la textura.
   *
   * Placeholder para lógica de actualización de texturas.
   */
  void
  update();

  /**
   * @brief Renderiza la textura en el pipeline gráfico.
   *
   * @param deviceContext Contexto del dispositivo de DirectX.
   * @param StartSlot Slot de inicio donde se asignará la textura.
   * @param NumView Número de vistas de recurso de shader a asignar.
   */
  void
  render(DeviceContext& deviceContext, unsigned int StartSlot, unsigned int NumView);

  /**
   * @brief Libera los recursos asociados a la textura.
   */
  void
  destroy();


public:
  /**
   * @brief Puntero al recurso de textura 2D en DirectX 11.
   */
  ID3D11Texture2D* m_texture = nullptr;

  /**
   * @brief Vista de recurso de shader creada a partir de la textura.
   */
  ID3D11ShaderResourceView* m_textureFromImg = nullptr;

  /**
   * @brief Nombre o ruta de la textura cargada.
   */
  std::string m_rextureName;
};
