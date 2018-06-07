#define BOOST_FILESYSTEM_VERSION 3

#ifndef BOOST_FILESYSTEM_NO_DEPRECATED 
#  define BOOST_FILESYSTEM_NO_DEPRECATED
#endif
#ifndef BOOST_SYSTEM_NO_DEPRECATED 
#  define BOOST_SYSTEM_NO_DEPRECATED
#endif

#include "boost/algorithm/string.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/progress.hpp"

#include <iostream>
#include <fstream>
#include <vector>

namespace fs = boost::filesystem;

std::string getDifPath(fs::path subdirs, fs::path pmain) {

	std::string dir = "";
	for (int i = pmain.string().length()+1; i < subdirs.string().length(); i++) {
		dir += subdirs.string()[i];
	}

	return dir;

}

void getListOfFilesInside(std::vector<std::string> &arr, fs::path current, fs::path realdir) {

	fs::directory_iterator end_iter;

	for (
		fs::directory_iterator iter(current);
		iter != end_iter;
		iter++
	) {
		if (fs::is_directory(iter->path())) {
		
			getListOfFilesInside(arr, iter->path(), realdir);
		
		} else {

			arr.push_back( getDifPath(iter->path(), realdir) );
		}
	}
}

void copyfile(fs::path from, fs::path todir, std::string directory) {

	//splitting directory
	std::vector<std::string> paths;
	boost::split(paths, directory, boost::is_any_of("/"));

	std::string dir = todir.string();

	//creating directories if they doesn't exists
	for (int i = 0; i < paths.size()-1; i++) {
		
		dir += "/" + paths[i];
		
		if (not fs::is_directory( fs::path(dir) )) {
			fs::create_directory( fs::path(dir) );
		}
	}

	//for overwriting
	if (fs::exists(todir/directory)) {
		fs::remove(todir/directory);
	}

	//copy
	fs::copy_file(from, todir/directory);
}

int main(int argc, char* argv[])
{
	
	fs::path path_from(fs::current_path());
	fs::path path_to(fs::current_path());

	if (argc > 2) {

		path_from = fs::system_complete(argv[1]);
		path_to = fs::system_complete(argv[2]);

		if (not fs::is_directory(path_from)) {

			std::cout << "[Error]: Main Directory is not exist" << std::endl;
			return 0;

		}

		if (not fs::is_directory(path_to)) {

			fs::create_directory(path_to);
		}

	
	} else {

		std::cout << "[Error]: Wrong Arguments" << std::endl;
		return 0;

	}

	//list of files
	std::vector<std::string> list_from;
	std::vector<std::string> list_to;

	getListOfFilesInside(list_from, path_from, path_from);
	getListOfFilesInside(list_to, path_to, path_to);

	std::sort(list_from.begin(), list_from.end());
	std::sort(list_to.begin(), list_to.end());

	std::vector<std::string> intersection;
	std::vector<std::string> removable;
	std::vector<std::string> newfiles;

	//get intersected files to copy
	for (int i = 0; i < list_to.size(); i++) {
		for (int j = 0; j < list_from.size(); j++) {
			if (list_from[j] == list_to[i]) {
				intersection.push_back(list_from[j]);
			}
		}
	}

	//get removable files
	if (intersection.size() > 0) {

		int i = 0;
		int j = 0;
		while (i+j < list_to.size()) {

			if (i+j > intersection.size()) {
				newfiles.push_back(list_to[i+j]);
				j++;
				continue;
			}

			if (list_to[i+j] != intersection[i]) {

				removable.push_back(list_to[i+j]);
				j++;

			} else {
				
				i++;
			}
		}
	
		i = 0;
		j = 0;
		while (i+j < list_from.size()) {

			if (i+j > intersection.size()) {
				newfiles.push_back(list_from[i+j]);
				j++;
				continue;
			}

			if (list_from[i+j] != intersection[i]) {

				newfiles.push_back(list_from[i+j]);
				j++;

			} else {
				
				i++;
			}
		}

	} else {

		newfiles = list_from;
	}

	std::cout << "Detected " << list_to.size() << " files in " << path_to << " :" << std::endl;

	//info block
	int k = 0;
	for(int i = 0; i < removable.size(); i++) {
		std::cout << ++k << ".\tTo Remove: \t" << path_to / removable[i] << std::endl;
	}

	for(int i = 0; i < newfiles.size(); i++) {
		std::cout << ++k << ".\tTo Copy: \t" << path_to / newfiles[i] << std::endl;
	}

	for(int i = 0; i < intersection.size(); i++) {
		std::cout << ++k << ".\tTo Replace: \t" << path_to / intersection[i] << std::endl;
	}

	std::cout << std::endl << "Starting:" << std::endl;

	//removing
	for(int i = 0; i < removable.size(); i++) {
		std::cout << "\tRemoving: \t" << path_to / removable[i] << std::endl;
		fs::remove(fs::path(path_to.string() + "/" + removable[i]));
	}

	//copy files
	for (int i = 0; i < newfiles.size(); i++) {
	
		std::cout << "\tCopy: \t\t" << path_to / newfiles[i] << std::endl;
		
		copyfile(
			path_from / newfiles[i],
			path_to,
			newfiles[i]
		);
	}

	//replace files
	for (int i = 0; i < intersection.size(); i++) {

		int l1 = fs::last_write_time(path_from / intersection[i]);
		int l2 = fs::last_write_time(path_to / intersection[i]);
	
		if (l1 > l2) {

			std::cout << "\tReplace: \t" << path_to / intersection[i] << std::endl;
			copyfile(
				path_from / intersection[i],
				path_to,
				intersection[i]
			);
		
		} else {

			std::cout << "\tUnchange: \t" << path_to / intersection[i] << std::endl;
		} 
	}


	return 0;
}