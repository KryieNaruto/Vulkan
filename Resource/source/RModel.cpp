#include "RModel.h"
#include "resource_inner_global.h"
#include <Core/include/core_global.h>
#include <shader_uniform_struct.h>
#include <set>
#include <thread>
#include <future>

namespace std {

	size_t std::hash<Core::Resource::RVertex*>::operator()(Core::Resource::RVertex* const& _v) const
	{
		size_t _hash_pos = hash<glm::vec3>()(_v->m_position);
		size_t _hash_normal = hash<glm::vec3>()(_v->m_normal);
		size_t _hash_tex = hash<glm::vec2>()(_v->m_texCoord);
		auto _rel = ((_hash_pos ^ (_hash_normal << 1)) >> 1) ^ (_hash_tex << 1);
		return _rel;
	}
}
bool equal::operator()(const Core::Resource::RVertex* _lhs, const Core::Resource::RVertex* _rhs) const
{
	return _lhs->m_position == _rhs->m_position &&
		_lhs->m_normal == _rhs->m_normal &&
		_lhs->m_texCoord == _rhs->m_texCoord;
}

Json::Value Core::Resource::RModel::serializeToJSON()
{
	return Json::Value();
}

void Core::Resource::RModel::deserializeToObj(Json::Value& _root)
{

}

Core::Resource::RModel::RModel(const std::string& _path)
{
	init_0();
	loadFromFile(_path);
}

Core::Resource::RModel::RModel()
{
	init_0();
}

Core::Resource::RModel::~RModel()
{
	if (!m_is_copy) {
		for (const auto& _vertex : p_vertices)
			delete _vertex;
		for (const auto& mesh : p_meshes)
			delete mesh;
		for (const auto& texture : p_textures)
			for (const auto& t : texture.second)
				delete t;
		/*for (const auto& [_input, _vertices] : p_vertices_in_shaders)
			free(_vertices);*/

	}
}

void Core::Resource::RModel::init_0()
{
	this->m_json_title = "RModel";
	this->p_id_generate->init(RESOURCE_TYPE::RMODEL);
	this->m_uuid = this->p_id_generate->generateUUID();
	this->p_property = new RMeshProperty;
	this->p_merge_textures[0] = {};
}

void Core::Resource::RModel::setup()
{
	if (p_root_scene) {
		processAllVertex();
		// ��ȡscene�����е�����
		processAllMaterial();
		// �ϲ�����
		mergeAllMaterial();
		// ����mesh
		loadModel();
		//// ��root node��ʼ����mesh
		//processNode(p_root_scene->mRootNode);
		this->m_initialized = true;
	}
}

void Core::Resource::RModel::processNode(aiNode* node)
{
	// ����mesh�ڵ�
	for (size_t _index = 0; _index < node->mNumMeshes; _index++) {
		aiMesh* mesh = p_root_scene->mMeshes[node->mMeshes[_index]];
		std::vector<uint32_t> _indices;	// ����
		std::vector<RVertex*> _vertices;
		// ��ȡ mesh ��������
		for (size_t _i = 0; _i < mesh->mNumVertices; _i++) {
			glm::vec3 _pos;
			{
				const auto& _v = mesh->mVertices[_i];
				_pos.x = _v.x;
				_pos.y = _v.y;
				_pos.z = _v.z;
			}
			glm::vec3 _normal; {
				const auto& _v = mesh->mNormals[_i];
				_normal.x = _v.x;
				_normal.y = _v.y;
				_normal.z = _v.z;
			}
			glm::vec2 _tex; {
				if (mesh->mTextureCoords[0]) {
					const auto& _v = mesh->mTextureCoords[0][_i];
					_tex.x = _v.x;
					_tex.y = _v.y;
				}
				else _tex = { 0,0 };
			}
			glm::vec3 _tangent; {
				const auto& _v = mesh->mTangents[_i];
				_tangent.x = _v.x;
				_tangent.y = _v.y;
				_tangent.z = _v.z;
			}
			glm::vec3 _bitangent; {
				const auto& _v = mesh->mBitangents[_i];
				_bitangent.x = _v.x;
				_bitangent.y = _v.y;
				_bitangent.z = _v.z;
			}
			std::string _title = std::string("RVertex").append(std::to_string(_i));
			auto _v = new RVertex(_pos, _normal, _tex, _tangent,_bitangent, _title);
			if (p_unique_vertices.find(_v) == p_unique_vertices.end()) {
				p_unique_vertices[_v] = p_vertices.size();
				p_vertices.push_back(_v);
			}
			_vertices.push_back(_v);
		}
		// ��ȡ��ǰmesh����
		for (size_t _face = 0; _face < mesh->mNumFaces; _face++) {
			aiFace _f = mesh->mFaces[_face];
			// ���ݶ����ȡ�ܶ�������
			auto _0 = _vertices[_f.mIndices[2]];
			auto _1 = _vertices[_f.mIndices[1]];
			auto _2 = _vertices[_f.mIndices[0]];
			auto _index_0 = p_unique_vertices[_0];
			auto _index_1 = p_unique_vertices[_1];
			auto _index_2 = p_unique_vertices[_2];
			// ������������˳ʱ�롣�ֶ���ʱ�����
			_indices.push_back(_index_0);
			_indices.push_back(_index_1);
			_indices.push_back(_index_2);
		}
		int _material_index = -1;
		// ��ȡ��ǰmesh ��������
		if (mesh->mMaterialIndex >= 0) {
			auto _mat_index = mesh->mMaterialIndex;
			_material_index = p_texture_To_merge_textures[_mat_index];
		}
		// ����mesh
		std::string _title = mesh->mName.C_Str();
		if (!_indices.empty()) {
			p_meshes.push_back(new RMesh(_indices, _material_index, _title));
		}
	}

	// �ݹ鴦��ǰ�ڵ��ӽڵ�
	for (size_t i = 0; i < node->mNumChildren; i++)
		processNode(node->mChildren[i]);
}

void Core::Resource::RModel::processAllMaterial()
{
	for (size_t i = 0; i < p_root_scene->mNumMaterials; i++) {
		aiMaterial* _mat = p_root_scene->mMaterials[i];
		// Diffuse
		const auto& _diffuse = loadMaterial(_mat, aiTextureType_DIFFUSE);
		// Specular
		const auto& _specular = loadMaterial(_mat, aiTextureType_SPECULAR);
		// Ambient
		const auto& _ambient = loadMaterial(_mat, aiTextureType_AMBIENT);

		p_textures[i] = {};
		if (!_diffuse.empty())
			p_textures[i].insert(p_textures[i].end(), _diffuse.begin(), _diffuse.end());
		if (!_specular.empty())
			p_textures[i].insert(p_textures[i].end(), _specular.begin(), _specular.end());
		if (!_ambient.empty())
			p_textures[i].insert(p_textures[i].end(), _ambient.begin(), _ambient.end());
	}
	// ������������
	for (const auto& [_mat_index, _textures] : p_textures) {
		for (const auto& _texture : _textures) {
			auto p_tex = ThirdParty::TextureEXT::getTextureEXT(_texture->getPath());
			if (p_tex == nullptr) {
				auto _path = _texture->getPath();
				p_tex = new ThirdParty::TextureEXT(_path);
			}
		}
	}
}

std::vector<Core::Resource::RTexture*> Core::Resource::RModel::loadMaterial(aiMaterial* mat, aiTextureType _type)
{
	std::vector<RTexture*> textures;
	for (uint32_t i = 0; i < mat->GetTextureCount(_type); i++) {
		aiString str;
		mat->GetTexture(_type, i, &str);
		std::filesystem::path _path = m_directroy + "\\" + str.C_Str();	// �����·��ת��Ϊ����·��
		_path = std::filesystem::absolute(_path);
		RTexture* p_t = new RTexture(_path.string().c_str(), _type, _path.string().c_str());
		textures.push_back(p_t);
	}
	return textures;
}

void Core::Resource::RModel::setupVkData()
{
	setupVkData(false, _default, 0);
}

void Core::Resource::RModel::setupVkData(bool _only_vertex_buffer, VERTEX_INPUT_FLAG input, uint32_t stride)
{
	if (m_initialized && isFinished()) {
		// vertex buffer, ����material��shader location ������vertex input
		VERTEX_INPUT_FLAG _input = input;
		size_t _stride = stride;
		if (p_shader) {
			if (!_only_vertex_buffer) {
				p_materials.clear();
				bool _first = true;
				// ����������vk����
				for (const auto& _tex_ite : p_merge_textures) {
					RMaterial* p_material = new RMaterial();
					p_material->setShader(p_shader)->bindTextures(_tex_ite.second);
					p_materials.push_back(p_material);
					// �����������
					if (p_property->m_texture_count < _tex_ite.second.size()) {
						p_property->m_texture_count = _tex_ite.second.size();
					}
					_stride = p_material->getVertexInputStride();
					_input = p_material->getVertexInputFlag();
				}
				// mesh ���ݲ���������ʼ��
				for (const auto& _mesh : p_meshes) {
					_mesh->setMaterial(p_materials[_mesh->getMaterialIndex()]);
				}
			}
			// ������Ǹ����壬������VertexInput
			if (!m_is_copy && _stride > 0) {
				if (p_vertices_in_shaders.find(_input) == p_vertices_in_shaders.end()) {
					size_t _size = p_vertices.size() * _stride;
					p_vertices_in_shader = malloc(_size);
					size_t _vertex_offset = 0;
					for (const auto& _vertex : p_vertices) {
						_vertex->pushData(_input, p_vertices_in_shader, _vertex_offset);
					}
					// ����buffer
					std::string title = std::format("{}:[Vertex Buffer]", m_res_name);
					p_vertex_buffer = new ThirdParty::VertexBuffer(_size, title);
					// ������
					p_vertex_buffer->bindData(p_vertices_in_shader);
					p_vertices_in_shaders[_input][this] = p_vertices_in_shader;
					p_vertex_buffers[_input][this] = p_vertex_buffer;
				}
				else {
					if (p_vertices_in_shaders[_input].find(this) == p_vertices_in_shaders[_input].end()) {
						size_t _size = p_vertices.size() * _stride;
						p_vertices_in_shader = malloc(_size);
						size_t _vertex_offset = 0;
						for (const auto& _vertex : p_vertices) {
							_vertex->pushData(_input, p_vertices_in_shader, _vertex_offset);
						}
						// ����buffer
						std::string title = std::format("{}:[Vertex Buffer]", m_res_name);
						p_vertex_buffer = new ThirdParty::VertexBuffer(_size, title);
						// ������
						p_vertex_buffer->bindData(p_vertices_in_shader);
						p_vertices_in_shaders[_input][this] = p_vertices_in_shader;
						p_vertex_buffers[_input][this] = p_vertex_buffer;
					}
					else {
						p_vertices_in_shader = p_vertices_in_shaders[_input][this];
						p_vertex_buffer = p_vertex_buffers[_input][this];
					}
				}
			}
		}
	}
}

Core::Resource::RModel* Core::Resource::RModel::setShader(Shader* _shader)
{
	if (_shader == nullptr) return this;
	p_shader = _shader;
	if (isFinished()) {
		setupVkData();
	}
	return this;
}

void Core::Resource::RModel::processAllVertex()
{
	if (p_root_scene) {
		// ��ȡ���ж��� ��ȥ��
		for (size_t i = 0; i < p_root_scene->mNumMeshes; i++) {
			auto _mesh = p_root_scene->mMeshes[i];
			// ��ȡ mesh ��������
			for (size_t _i = 0; _i < _mesh->mNumVertices; _i++) {
				glm::vec3 _pos;
				{
					const auto& _v = _mesh->mVertices[_i];
					_pos.x = _v.x;
					_pos.y = _v.y;
					_pos.z = _v.z;
				}
				glm::vec3 _normal; {
					const auto& _v = _mesh->mNormals[_i];
					_normal.x = _v.x;
					_normal.y = _v.y;
					_normal.z = _v.z;
				}
				glm::vec2 _tex; {
					if (_mesh->mTextureCoords[0]) {
						const auto& _v = _mesh->mTextureCoords[0][_i];
						_tex.x = _v.x;
						_tex.y = _v.y;
					}
					else _tex = { 0,0 };
				}
				glm::vec3 _tangent; {
					const auto& _v = _mesh->mTangents[_i];
					_tangent.x = _v.x;
					_tangent.y = _v.y;
					_tangent.z = _v.z;
				}
				glm::vec3 _bitangent; {
					const auto& _v = _mesh->mBitangents[_i];
					_bitangent.x = _v.x;
					_bitangent.y = _v.y;
					_bitangent.z = _v.z;
				}
				std::string _title = std::string("RVertex").append(std::to_string(_i));
				RVertex* p_vertex = new RVertex(_pos, _normal, _tex,_tangent,_bitangent, _title);
				if (p_unique_vertices.find(p_vertex) == p_unique_vertices.end()) {
					p_unique_vertices[p_vertex] = p_vertices.size();
					p_vertices.push_back(p_vertex);
				}
				else delete p_vertex;
			}
		}
	}
}

void Core::Resource::RModel::mergeAllMaterial()
{
	std::unordered_set<std::vector<RTexture*>> _has_load;
	uint32_t _index = 0;
	for (const auto& [_mat_index, _textures] : p_textures) {
		// δ���ֹ�
		if (!_has_load.contains(_textures)) {
			_has_load.insert(_textures);
			p_merge_textures[_index] = _textures;
			p_texture_To_merge_textures[_mat_index] = _index;
			_index++;
		}
	}
}

void Core::Resource::RModel::loadModel()
{
	auto _root_node = p_root_scene->mRootNode;
	if (_root_node->mNumChildren == 0) {
		std::async(std::launch::async, [this](aiNode* node) {
			processNode_thread(node);
			}, _root_node);
	}
	else {
		for (size_t _index = 0; _index < _root_node->mNumChildren; _index++) {
			std::async(std::launch::async, [this](aiNode* node) {
				processNode_thread(node);
				}, _root_node->mChildren[_index]);
		}
	}
}

void Core::Resource::RModel::processNode_thread(aiNode* node)
{
	// ����mesh�ڵ�
	for (size_t _index = 0; _index < node->mNumMeshes; _index++) {
		aiMesh* mesh = p_root_scene->mMeshes[node->mMeshes[_index]];
		std::vector<uint32_t> _indices;	// ����
		std::vector<RVertex*> _vertices;
		// ��ȡ mesh ��������
		for (size_t _i = 0; _i < mesh->mNumVertices; _i++) {
			glm::vec3 _pos;
			{
				const auto& _v = mesh->mVertices[_i];
				_pos.x = _v.x;
				_pos.y = _v.y;
				_pos.z = _v.z;
			}
			glm::vec3 _normal; {
				const auto& _v = mesh->mNormals[_i];
				_normal.x = _v.x;
				_normal.y = _v.y;
				_normal.z = _v.z;
			}
			glm::vec2 _tex; {
				if (mesh->mTextureCoords[0]) {
					const auto& _v = mesh->mTextureCoords[0][_i];
					_tex.x = _v.x;
					_tex.y = _v.y;
				}
				else _tex = { 0,0 };
			}
			glm::vec3 _tangent; {
				const auto& _v = mesh->mTangents[_i];
				_tangent.x = _v.x;
				_tangent.y = _v.y;
				_tangent.z = _v.z;
			}
			glm::vec3 _bitangent; {
				const auto& _v = mesh->mBitangents[_i];
				_bitangent.x = _v.x;
				_bitangent.y = _v.y;
				_bitangent.z = _v.z;
			}
			std::string _title = std::string("RVertex").append(std::to_string(_i));
			auto _v = new RVertex(_pos, _normal, _tex, _tangent,_bitangent, _title);
			_vertices.push_back(_v);
		}
		// ��ȡ��ǰmesh����
		for (size_t _face = 0; _face < mesh->mNumFaces; _face++) {
			aiFace _f = mesh->mFaces[_face];
			// ���ݶ����ȡ�ܶ�������
			auto _0 = _vertices[_f.mIndices[2]];
			auto _1 = _vertices[_f.mIndices[1]];
			auto _2 = _vertices[_f.mIndices[0]];
			auto _index_0 = p_unique_vertices[_0];
			auto _index_1 = p_unique_vertices[_1];
			auto _index_2 = p_unique_vertices[_2];
			// ������������˳ʱ�롣�ֶ���ʱ�����
			_indices.push_back(_index_0);
			_indices.push_back(_index_1);
			_indices.push_back(_index_2);
		}
		int _material_index = -1;
		// ��ȡ��ǰmesh ��������
		if (mesh->mMaterialIndex >= 0) {
			auto _mat_index = mesh->mMaterialIndex;
			_material_index = p_texture_To_merge_textures[_mat_index];
			//_material_index = _mat_index;
		}
		// ����mesh
		std::string _title = mesh->mName.C_Str();
		if (!_indices.empty()) {
			mtx.lock();
			p_meshes.push_back(new RMesh(_indices, _material_index, _title));
			// ������ص�Mesh�Ѿ������ڵ�����ޣ���˵��������ϡ�
			if (p_meshes.size() == p_root_scene->mNumMeshes)
				m_loading_finished = true;
			mtx.unlock();
		}
	}

	// �ݹ鴦��ǰ�ڵ��ӽڵ�
	for (size_t i = 0; i < node->mNumChildren; i++) {
		std::async(std::launch::async, [this](aiNode* node) {
			processNode_thread(node);
			}, node->mChildren[i]);
	}
}

bool Core::Resource::RModel::isFinished()
{
	if (p_root_scene) {
		return m_loading_finished;
	}
	else
		return true;
}

void Core::Resource::RModel::initTemplate()
{

}

void Core::Resource::RModel::loadFromFile(const std::string& _path)
{
	const auto& path = getResourcePath(_path);
	p_root_scene = ThirdParty::assimp_manager.getModel(path);
	if (p_root_scene) {
		m_path = path;
		this->m_res_name = p_root_scene->mName.C_Str();
		if (m_res_name == "")
			this->m_res_name = std::filesystem::path(m_path).filename().string();
		m_directroy = g_file_manager->getFilePath(m_path);
		setup();
		Core::Core_Pool::addResource(m_uuid, this);
	}
}

void Core::Resource::RModel::loadFromModel(const RModel* _p)
{
	this->m_is_copy = true;
	this->m_initialized = _p->m_initialized;
	this->m_loading_finished = _p->m_loading_finished;
	this->p_shader = _p->p_shader;
	this->p_merge_textures = _p->p_merge_textures;
	this->p_vertex_buffer = _p->p_vertex_buffer;
	this->p_meshes = _p->p_meshes;
	// ����Merge���Material
	setupVkData();
}
