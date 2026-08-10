# Sistema de Partículas Basado en SDF (DirectX 11)

**Autor:** José Sebastian Cardoza Escobar[cite: 1, 2]
**Proyecto Final 9no Cuatrimestre** - *Gráficas Computacionales 3D y Programación de Materiales*[cite: 1, 2]

<img width="1917" height="989" alt="Captura de pantalla 2026-08-09 193800" src="https://github.com/user-attachments/assets/bd6e1ba2-9051-417c-a9da-01e44a2545bf" />

## Visión General del Sistema
Este proyecto es un módulo de renderizado diseñado para generar efectos visuales dinámicos de alto rendimiento en tiempo real, operando eficientemente[cite: 2]. Su principal innovación es la eliminación de la dependencia de texturas externas mediante el uso de matemáticas puras con *Signed Distance Fields* (SDF)[cite: 1, 2]. 

Esta ventaja técnica reduce drásticamente el consumo de memoria de video (VRAM) y el ancho de banda, al mismo tiempo que garantiza mantener bordes infinitamente nítidos a cualquier escala sin degradación[cite: 1, 2].

## Arquitectura y Paradigma Base
El sistema está construido bajo el paradigma de *Entity Component System* (ECS) mediante el componente clave `ParticleEmitterComponent`[cite: 1, 2]. El diseño divide su carga de procesamiento en dos etapas fundamentales[cite: 1]:

<img width="762" height="451" alt="Captura de pantalla 2026-08-09 195533" src="https://github.com/user-attachments/assets/b20bad5c-89bc-4a95-a36f-01c78120cc1d" />

### 1. Simulación en CPU (Cinemática y Ciclo de Vida)
* Gestiona la cinemática, el ciclo de vida y el reciclaje estricto de memoria para asegurar estabilidad[cite: 2].
* Aplica física básica mediante integración de Euler explícita, afectando el eje vertical de la velocidad mediante gravedad[cite: 2].
* Utiliza un factor de tiempo normalizado para interpolar linealmente (Lerp) de manera progresiva el tamaño geométrico y los componentes de color RGB hasta la muerte de la partícula[cite: 2].

### 2. Renderizado en GPU (Pipeline y Billboarding)
* La CPU envía 6 vértices idénticos por cada partícula, delegando la expansión pesada al hardware de la GPU[cite: 2].
* El Vertex Shader aplica técnicas de *Spherical Billboarding* dinámico extrayendo los vectores Right y Up desde la Matriz de Vista, omitiendo funciones trigonométricas[cite: 2].
* El Vertex Buffer se actualiza exhaustivamente en cada fotograma mediante la directiva `D3D11_MAP_WRITE_DISCARD` para descartar datos obsoletos y evitar colapsos por sincronización[cite: 2].

## Proceduralismo Matemático (SDF)
En lugar de muestrear imágenes costosas, el Pixel Shader evalúa matemáticamente las formas directamente en la tarjeta gráfica[cite: 1, 2]. 

<img width="1339" height="356" alt="Captura de pantalla 2026-08-09 195224" src="https://github.com/user-attachments/assets/2350a324-c032-45ae-b9d5-5f38ecef6fda" />

Se utiliza el descarte algorítmico de fragmentos usando la función `smoothstep(0.4, 0.9, d)` para lograr un contorno perfecto[cite: 2]. Además, se aplica una suma matemática de los valores RGB de partículas concurrentes (Mezcla Aditiva / BlendOp_ADD) simulando la acumulación de luz[cite: 2]. Se deshabilita activamente la escritura en el Z-buffer, conservando la lectura para que las entidades se oculten tras la geometría sin recortarse entre sí de forma cuadrada[cite: 2].

## Optimización Extrema
Para evitar las caídas de rendimiento críticas (fragmentación) provocadas por la instanciación dinámica, el sistema utiliza pre-reserva estática de memoria[cite: 2].

<img width="670" height="491" alt="Captura de pantalla 2026-08-09 195308" src="https://github.com/user-attachments/assets/a16d4e95-1f2b-4674-9d35-2714ae61f925" />

* **Object Pooling Activo:** Se reserva un arreglo de partículas (buffer de 1000 partículas contiguas por defecto) en la fase inicial[cite: 1, 2].
* **Reciclaje Lógico:** Las unidades sin tiempo de vida son apagadas e ignoradas; las partículas se sobrescriben en lugar de ser creadas o destruidas, evitando operaciones pesadas[cite: 2].

## Integración con el Editor (Dear ImGui)
El sistema proporciona herramientas visuales construidas sobre Dear ImGui para brindar un flujo iterativo a los artistas técnicos[cite: 2].

<img width="1478" height="862" alt="Captura de pantalla 2026-08-09 192855" src="https://github.com/user-attachments/assets/899aa81b-2e26-44fe-9d34-487c4e261424" />

* **Diseño Adaptativo:** Ocultamiento dinámico de parámetros que resultan irrelevantes según el contexto geométrico seleccionado[cite: 2].
* **Sistema de Presets:** Botones de acción rápida para inyectar perfiles calibrados instantáneamente como Fuego, Chispas, Humo o Burbujas[cite: 2]. El sistema ajusta inteligentemente múltiples parámetros interconectados simultáneamente para brindar una base estructural lista para trabajar[cite: 2].
