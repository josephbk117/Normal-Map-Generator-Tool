# Nora, The Normal Map Generator Tool
It's a normal map generator / editor tool.
### Get the latest version here : [Nora-Normal Map Editor](https://github.com/josephbk117/Normal-Map-Generator-Tool/releases) 

Takes height map information which is usually gray scale ( Height map data ) and converts that data into normal map data which then can be further edited and exported.

* Minimal and portable
* Image formats supported : PNG, JPG, BMP & TGA
* Model formats supported : OBJ & FBX
* Custom brushes can be added easily by adding them in the Resources\Brushes
* Custom themes can be added by adding custom .nort theme files in Resources\Themes
* Extensive preview capabilities

*Disclaimer : Can be used in production but look out for the rare bug here and there*

### Software as of version 1.0 Beta
![Image of Nora normal map editor](https://i.imgur.com/1fHhvXA.png)
#### Different view modes are available while editing the normal map :
1. Height( Gray scale )
2. Normal
3. Lighting

Realtime preview of the texture applied on a model can be shown as well.
This is a real-time preview of how the texture will look once applied on the model.
#### This preview mode has 4 modes:
1. Height
2. Normal
3. Lighting
4. Textured, which combines lighting along with a custom diffuse texture.

![Image of Nora normal map editor](https://i.imgur.com/BoIjEQj.png)

## Roadmap of upcoming features:
- Editor Functionality:
  - Better brush drawing performance and functionality
  - Blur improvements
  - Adding more brushes in brush library
  - Display brush outline better
  - Adding layers:
    - Creation of layers and layers groups
    - Adding blend options to interpolate normals between layers    
- Preview Display:
  - Adding support for parallax map using the input height map
  - Adding PBR support along with Metalness map, Roughness map & Occlusion map
  - Custom shader display option ( Allows custom viewing of model in preview )
  - Better user interaction with preview UI
  - Adding more base models for preview
  - Additional lighting options:
    - Directional light with option for colour and strength of light as well as visual indication of the directional light
    - Having a point light ( Only 1 ) with linear and exponential attenuation multiplier customization along with colour and strength 
    - Adding IBL ( Image based lighting )
- Covering more map types ( Technically, Nora will always be a normal map editor at heart :grin: ):
  - Cavity map [Non-PBR workflow]
  - Specular map [Non-PBR workflow]
  - Ambient Occlusion map [PBR workflow]( Take height map data and normal map data to create realistic results )
  - Metalness map [PBR workflow]
  - Roughness map [PBR workflow]
- Ease of use:
  - Adding preferences window:
    - Set max resolution image that can be edited ( Larger max image resolution takes more RAM )
    - Adding max number undo steps
    - Set the default theme
    - Set the default export path
  - Add any file for models / brush textures / theme files (.nort) to application's library. These ones will get loaded up along with the base assets
  - Cover more application edge cases and report correct information to user. eg: Running out of memory, space etc
  - Export options:
    - Select export size
    - Format of image
    - Export image quality ( for formats that apply )
    - Export map grouping (examples):
      - Can export normal map data in RGB channel and height map data in Alpha channel
      - Metalness in Red channel, Roughness in Blue and AO in Green channel
      - etc...

*If you like the work being done please do support in terms of feature suggestions and bug reporting*
