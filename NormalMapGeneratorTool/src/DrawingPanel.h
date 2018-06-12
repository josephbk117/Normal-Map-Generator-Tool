#pragma once
class DrawingPanel
{
public:
	DrawingPanel();
	~DrawingPanel();
	void init(float width, float height);
	void setTextureID(unsigned int textureID);
	unsigned int getTextureID()const noexcept;
	void draw();
private:
	float width, height;
	unsigned int vaoID;
	unsigned int vboID;
	unsigned int textureID;
};

