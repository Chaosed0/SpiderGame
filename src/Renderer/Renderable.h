
#include <vector>
#include <glm/glm.hpp>

enum DrawMethod
{
	DrawMethod_Elements,
	DrawMethod_Arrays
};

class Renderable
{
public:
	virtual unsigned getVao() = 0;
	virtual unsigned getVertexCount() = 0;
	virtual unsigned getIndexCount() = 0;
	virtual int getDrawMode() = 0;

	virtual bool isAnimatable() = 0;
	virtual std::vector<glm::mat4> getBoneTransforms() = 0;
};