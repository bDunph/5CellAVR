#include <string>


#ifdef __APPLE__ 
#include <GL/glew.h>
#include "GLFW/glfw3.h"
#include "openvr/openvr.h"
#elif _WIN32 
#include "GL/glew.h"
#include "glfw3.h"
#include "openvr.h"
#endif

class CGLRenderModel
{
public:
	CGLRenderModel( const std::string & sRenderModelName );
	~CGLRenderModel();

	bool BInit( const vr::RenderModel_t & vrModel, const vr::RenderModel_TextureMap_t & vrDiffuseTexture );
	void Cleanup();
	void Draw();
	const std::string & GetName() const { return m_sModelName; }

private:
	GLuint m_glVertBuffer;
	GLuint m_glIndexBuffer;
	GLuint m_glVertArray;
	GLuint m_glTexture;
	GLsizei m_unVertexCount;
	std::string m_sModelName;
};
