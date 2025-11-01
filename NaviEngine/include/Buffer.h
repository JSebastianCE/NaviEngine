#pragma once
#include "Prerequisites.h"
#include "MeshComponent.h"

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
 * @class Buffer
 * @brief Clase encargada de manejar la creaci�n, actualizaci�n, renderizado
 *        y destrucci�n de buffers utilizados en DirectX.
 */
class
Buffer {
public:
  /**
   * @brief Constructor por defecto de la clase Buffer.
   */
  Buffer() = default;

  /**
   * @brief Destructor por defecto de la clase Buffer.
   */
  ~Buffer() = default;

  /**
   * @brief Inicializa el buffer utilizando los datos de un componente de malla.
   * @param device Referencia al dispositivo de renderizado.
   * @param mesh Referencia al componente de malla que contiene los datos del modelo.
   * @param bindFlag Indicador de tipo de enlace (por ejemplo, v�rtices o �ndices).
   * @return HRESULT que indica el resultado de la operaci�n.
   */
  HRESULT
  init(Device& device, const MeshComponent& mesh, unsigned int bindFlag);

  /**
   * @brief Inicializa un buffer vac�o con un tama�o en bytes determinado.
   * @param device Referencia al dispositivo de renderizado.
   * @param ByteWidth Tama�o del buffer en bytes.
   * @return HRESULT que indica el resultado de la creaci�n.
   */
  HRESULT
  init(Device& device, unsigned int ByteWidth);

  /**
   * @brief Actualiza el contenido del buffer con nuevos datos.
   * @param deviceContext Contexto del dispositivo para la actualizaci�n.
   * @param pDstResource Recurso de destino a actualizar.
   * @param DstSubresource �ndice del subrecurso de destino.
   * @param pDstBox Caja que define el �rea del recurso a actualizar.
   * @param pSrcData Puntero a los datos fuente.
   * @param SrcRowPitch Tama�o de la fila de datos fuente.
   * @param SrcDepthPitch Tama�o de la profundidad de los datos fuente.
   */
  void
  update(DeviceContext& deviceContext,
        ID3D11Resource* pDstResource,
        unsigned int    DstSubresource,
        const D3D11_BOX* pDstBox,
        const void* pSrcData,
        unsigned int    SrcRowPitch,
        unsigned int    SrcDepthPitch);

  /**
   * @brief Asocia el buffer al pipeline para su renderizado.
   * @param deviceContext Contexto del dispositivo para el renderizado.
   * @param StartSlot Posici�n inicial del buffer en el pipeline.
   * @param NumBuffers N�mero de buffers a establecer.
   * @param setPixelShader Indica si el buffer se usa tambi�n en el pixel shader.
   * @param format Formato DXGI utilizado por el buffer (opcional).
   */
  void
  render(DeviceContext& deviceContext,
          unsigned int   StartSlot,
          unsigned int   NumBuffers,
          bool           setPixelShader = false,
          DXGI_FORMAT    format = DXGI_FORMAT_UNKNOWN);

  /**
   * @brief Libera los recursos asociados al buffer.
   */
  void
  destroy();

  /**
   * @brief Crea un buffer con la descripci�n y los datos iniciales proporcionados.
   * @param device Referencia al dispositivo de renderizado.
   * @param desc Descripci�n del buffer (tipo, tama�o, uso, etc.).
   * @param initData Datos iniciales para llenar el buffer (puede ser nullptr).
   * @return HRESULT que indica el resultado de la operaci�n.
   */
  HRESULT
  createBuffer(Device& device,
                D3D11_BUFFER_DESC& desc,
                D3D11_SUBRESOURCE_DATA* initData);

private:

  /** @brief Puntero al buffer de Direct3D. */
  ID3D11Buffer* m_buffer = nullptr;

  /** @brief Tama�o en bytes de cada elemento del buffer (stride). */
  unsigned int m_stride = 0;

  /** @brief Desplazamiento en bytes desde el inicio del buffer. */
  unsigned int m_offset = 0;

  /** @brief Bandera que indica el tipo de enlace (bind flag) del buffer. */
  unsigned int m_bindFlag = 0;

};
