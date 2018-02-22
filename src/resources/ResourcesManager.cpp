#include "ResourcesManager.hpp"
#include "MeshUtilities.hpp"
#include "Logger.hpp"
#include <fstream>
#include <sstream>
#include <tinydir/tinydir.h>


/// Singleton.
Resources& Resources::manager(){
	static Resources* res = new Resources("resources");
	return *res;
}

Resources::Resources(const std::string & root) : _rootPath(root){
	// Parse directory for all files contained in it and its subdirectory.
	parseDirectory(_rootPath);
}


Resources::~Resources(){ }


void Resources::parseDirectory(const std::string & directoryPath){
	// Open directory.
	tinydir_dir dir;
	if(tinydir_open(&dir, directoryPath.c_str()) == -1){
		tinydir_close(&dir);
		Log::Error() << Log::Resources << "Unable to open resources directory at path \"" << directoryPath << "\"" << std::endl;
	}
	
	// For each file in dir.
	while (dir.has_next) {
		tinydir_file file;
		if(tinydir_readfile(&dir, &file) == -1){
			// Handle any read error.
			Log::Error() << Log::Resources << "Error getting file in directory \"" << std::string(dir.path) << "\"" << std::endl;
			
		} else if(file.is_dir){
			// Extract subdirectory name, check that it isn't a special dir, and recursively aprse it.
			const std::string dirName(file.name);
			if(dirName.size() > 0 && dirName != "." && dirName != ".."){
				// @CHECK: "/" separator on Windows.
				parseDirectory(directoryPath + "/" + dirName);
			}
			
		} else {
			// Else, we have a regular file.
			const std::string fileNameWithExt(file.name);
			// Filter empty files and system files.
			if(fileNameWithExt.size() > 0 && fileNameWithExt.at(0) != '.' ){
				if(_files.count(fileNameWithExt) == 0){
					// Store the file and its path.
					// @CHECK: "/" separator on Windows.
					_files[fileNameWithExt] = std::string(dir.path) + "/" + fileNameWithExt;
					
				} else {
					// If the file already exists somewhere else in the hierarchy, warn about this.
					Log::Error() << Log::Resources << "Error: asset named \"" << fileNameWithExt << "\" alread exists." << std::endl;
				}
			}
		}
		// Get to next file.
		if (tinydir_next(&dir) == -1){
			// Reach end of dir early.
			break;
		}
		
	}
	tinydir_close(&dir);
}


const std::shared_ptr<ProgramInfos> Resources::getProgram(const std::string & name){
	return getProgram(name, name, name);
}

const std::shared_ptr<ProgramInfos> Resources::getProgram(const std::string & name, const std::string & vertexName, const std::string & fragmentName) {
	if (_programs.count(name) > 0) {
		return _programs[name];
	}

	_programs.emplace(std::piecewise_construct,
		std::forward_as_tuple(name),
		std::forward_as_tuple(new ProgramInfos(vertexName, fragmentName)));

	return _programs[name];
}


const std::string Resources::getShader(const std::string & name, const ShaderType & type){
	
	std::string path = "";
	const std::string extension = type == Vertex ? "vert" : "frag";
	// Directly query correct shader text file with extension.
	const std::string res = Resources::getTextFile(name + "." + extension);
	// If the file is empty/doesn't exist, error.
	if(res.empty()){
		Log::Error() << Log::Resources << "Unable to find " << (type == Vertex ? "vertex" : "fragment") << " shader named \"" << name << "\"." << std::endl;
	}
	return res;
}

const MeshInfos Resources::getMesh(const std::string & name){
	if(_meshes.count(name) > 0){
		return _meshes[name];
	}

	MeshInfos infos;

	std::string path;
	// For now we only support OBJs.
	// Check if the file exists with an OBJ extension.
	if(_files.count(name + ".obj") > 0){
		path = _files[name + ".obj"];
	} else {
		Log::Error() << Log::Resources << "Unable to find mesh named \"" << name << "\"" << std::endl;
		// Return empty mesh.
		return infos;
	}
	
	// Load geometry.
	Mesh mesh;
	
	std::ifstream infile;
	infile.open(path.c_str());
	if(infile.is_open()){
		MeshUtilities::loadObj(infile, mesh, MeshUtilities::Indexed);
		// We are done with the file.
		infile.close();
		// If uv or positions are missing, tangent/binormals won't be computed.
		MeshUtilities::computeTangentsAndBinormals(mesh);
		
	} else {
		Log::Error() << Log::Resources << "Unable to load mesh at path " << path << "." << std::endl;
	}
	
	// Setup GL buffers and attributes.
	infos = GLUtilities::setupBuffers(mesh);
	_meshes[name] = infos;
	return infos;
}

const TextureInfos Resources::getTexture(const std::string & name, bool srgb){
	
	// If texture already loaded, return it.
	if(_textures.count(name) > 0){
		return _textures[name];
	}
	// Else, find the corresponding file.
	TextureInfos infos;
	std::string path = getImagePath(name);
	
	if(!path.empty()){
		// Else, load it and store the infos.
		infos = GLUtilities::loadTexture({path}, srgb);
		_textures[name] = infos;
		return infos;
	}
	// Else, maybe there are custom mipmap levels.
	// In this case the true name is name_mipmaplevel.
	
	// How many mipmap levels can we accumulate?
	std::vector<std::string> paths;
	unsigned int lastMipmap = 0;
	std::string mipmapPath = getImagePath(name + "_" + std::to_string(lastMipmap));
	while(!mipmapPath.empty()) {
		// Transfer them to the final paths vector.
		paths.push_back(mipmapPath);
		++lastMipmap;
		mipmapPath = getImagePath(name + "_" + std::to_string(lastMipmap));
	}
	if(!paths.empty()){
		// We found the texture files.
		// Load them and store the infos.
		infos = GLUtilities::loadTexture(paths, srgb);
		_textures[name] = infos;
		return infos;
	}
	
	// If couldn't file the image, return empty texture infos.
	Log::Error() << Log::Resources << "Unable to find texture named \"" << name << "\"." << std::endl;
	return infos;
}


const TextureInfos Resources::getCubemap(const std::string & name, bool srgb){
	// If texture already loaded, return it.
	if(_textures.count(name) > 0){
		return _textures[name];
	}
	// Else, find the corresponding files.
	TextureInfos infos;
	std::vector<std::string> paths = getCubemapPaths(name);
	if(!paths.empty()){
		// We found the texture files.
		// Load them and store the infos.
		infos = GLUtilities::loadTextureCubemap({paths}, srgb);
		_textures[name] = infos;
		return infos;
	}
	// Else, maybe there are custom mipmap levels.
	// In this case the true name is name_mipmaplevel.
	
	// How many mipmap levels can we accumulate?
	std::vector<std::vector<std::string>> allPaths;
	unsigned int lastMipmap = 0;
	std::vector<std::string> mipmapPaths = getCubemapPaths(name + "_" + std::to_string(lastMipmap));
	while(!mipmapPaths.empty()) {
		// Transfer them to the final paths vector.
		allPaths.push_back(mipmapPaths);
		++lastMipmap;
		mipmapPaths = getCubemapPaths(name + "_" + std::to_string(lastMipmap));
	}
	if(!allPaths.empty()){
		// We found the texture files.
		// Load them and store the infos.
		infos = GLUtilities::loadTextureCubemap(allPaths, srgb);
		_textures[name] = infos;
		return infos;
	}
	Log::Error() << Log::Resources << "Unable to find cubemap named \"" << name << "\"." << std::endl;
	// Nothing found, return empty texture.
	return infos;
}

const std::string Resources::getTextFile(const std::string & filename){
	std::string path = "";
	if(_files.count(filename) > 0){
		path = _files[filename];
	} else if(_files.count(filename + ".txt") > 0){
		path = _files[filename + ".txt"];
	} else {
		Log::Error() << Log::Resources << "Unable to find text file named \"" << filename << "\"." << std::endl;
		return "";
	}
	return Resources::loadStringFromFile(path);
}

void Resources::reload() {
	for (auto & prog : _programs) {
		prog.second->reload();
	}
	Log::Info() << Log::Resources << "Shader programs reloaded." << std::endl;
}

std::string Resources::trim(const std::string & str, const std::string & del){
	const size_t firstNotDel = str.find_first_not_of(del);
	if(firstNotDel == std::string::npos){
		return "";
	}
	const size_t lastNotDel = str.find_last_not_of(del);
	return str.substr(firstNotDel, lastNotDel - firstNotDel + 1);
}


const std::vector<std::string> Resources::getCubemapPaths(const std::string & name){
	const std::vector<std::string> names { name + "_px", name + "_nx", name + "_py", name + "_ny", name + "_pz", name + "_nz" };
	std::vector<std::string> paths;
	paths.reserve(6);
	for(auto & faceName : names){
		const std::string filePath = getImagePath(faceName);
		// If a face is missing, cancel the whole loading.
		if(filePath.empty()){
			return std::vector<std::string>();
		}
		// Else append the path.
		paths.push_back(filePath);
	}
	return paths;
}

const std::string Resources::getImagePath(const std::string & name){
	std::string path = "";
	// Check if the file exists with an image extension.
	if(_files.count(name + ".png") > 0){
		path = _files[name + ".png"];
	} else if(_files.count(name + ".jpg") > 0){
		path = _files[name + ".jpg"];
	} else if(_files.count(name + ".jpeg") > 0){
		path = _files[name + ".jpeg"];
	} else if(_files.count(name + ".bmp") > 0){
		path = _files[name + ".bmp"];
	} else if(_files.count(name + ".tga") > 0){
		path = _files[name + ".tga"];
	} else if(_files.count(name + ".exr") > 0){
		path = _files[name + ".exr"];
	}
	return path;
}

std::string Resources::loadStringFromFile(const std::string & filename) {
	std::ifstream in;
	// Open a stream to the file.
	in.open(filename.c_str());
	if (!in) {
		Log::Error() << Log::Resources << "" << filename + " is not a valid file." << std::endl;
		return "";
	}
	std::stringstream buffer;
	// Read the stream in a buffer.
	buffer << in.rdbuf();
	// Create a string based on the content of the buffer.
	std::string line = buffer.str();
	in.close();
	return line;
}


