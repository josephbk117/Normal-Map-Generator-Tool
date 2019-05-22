# Nora, The Normal Map Generator Tool
It's a normal map generator / editor tool.
### Get the latest version here : [Nora Normal Map Editor](https://github.com/josephbk117/Normal-Map-Generator-Tool/releases) 

Takes height map information which is usually gray scale ( Height map data ) and converts that data into normal map data which then can be further edited and exported.

* Minimal and portable
* Image formats supported : PNG, JPG, BMP & TGA
* Model formats supported : OBJ & FBX
* Custom brushes can be added easily by adding them in the Resources\Brushes
* Custom themes can be added by adding custom .nort theme files in Resources\Themes
* Extensive preview capabilities:
  * Matcaps
  * PBR material display

*Disclaimer : Can be used in production but look out for the rare bug here and there*

### Image of Nora as of version 1.2 Beta [Current version 1.4]
![Image of Nora normal map editor](https://i.imgur.com/JujvJES.png)
#### Different view modes are available while editing the normal map :
1. Height( Gray scale )
2. Normal
3. Lighting

Realtime preview of the texture applied on a model can be shown as well.
This is a real-time preview of how the texture will look once applied on the model.
#### This preview mode has 4 modes:
1. Height
2. Normal, Visualize the normals in various ways with the help of Matcaps
   ![Image of Nora Matcap Previewa](https://i.imgur.com/wI0svNZ.png)
3. Lighting, which shows PBR output with the help of Albedo, Metalness & Roughness textures

#### Custom theme support:
1. Create a .nort file with a certain format and add it under the Resources\Themes
2. Look at existing theme files to make your own
<div><img src="https://i.imgur.com/7ItkGf0.png" alt="alt text" width="300"/> <img src="https://i.imgur.com/mkSqdWF.png" alt="alt text" width="300"/><img src="https://i.imgur.com/h2Hlrym.png" alt="alt text" width="300"/><img src="https://i.imgur.com/OZPJzUA.png" alt="alt text" width="300"/></div>

#### Save editing progress
1. You can now save your intermediate work with .nora file
2. Saves all the layer data as well the main height map information

## Planned / Completed features as of version 1.4 :
- Editor Functionality:
  - Better brush drawing performance and functionality
  - Blur improvements
  - Adding more brushes in brush library
  - Display brush outline better
  - [X] Adding layers (completed as of v1.3):
    - [X] Creation of layers and layers groups
    - [X] Adding blend options to interpolate normals between layers    
- Preview Display:
  - Adding support for parallax map using the input height map
  - [X] Adding PBR support along with Metalness map, Roughness map & Occlusion map ( completed as of v1.2 )
  - Custom shader display option ( Allows custom viewing of model in preview )
  - [X] Better user interaction with preview UI (completed as of v1.2)
  - [X] Adding more base models for preview (completed as of v1.1)
  - Adding infinite plane with shadows in Lighting / Textured view ( Which can be toggled )
  - Additional lighting options:
    - [X] Directional light with option for colour and strength of light as well as visual indication of the directional light (completed as of v1.1)
    - Having a point light ( Only 1 ) with linear and exponential attenuation multiplier customization along with colour and strength 
    - Adding IBL ( Image based lighting )
- Covering more map types ( Technically, Nora will always be a normal map editor at heart :grin: ):
  - Cavity map [Non-PBR workflow]
  - Specular map [Non-PBR workflow]
  - Ambient Occlusion map [PBR workflow]( Take height map data and normal map data to create realistic results )
  - Metalness map [PBR workflow]
  - Roughness map [PBR workflow]
- Ease of use:
  - File explorer improvements:
    - Display icon based on file type
    - Once file selected detailed information is shown:
      - File size
      - File type
      - Last modified time
      - If image file then preview shown
  - [X] Adding preferences window (completed as of v1.1):
    - [X] Set max resolution image that can be edited ( Larger max image resolution takes more RAM )
    - [X] Adding max number undo steps
    - [X] Set the default theme
    - [X] Set the default export path
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
