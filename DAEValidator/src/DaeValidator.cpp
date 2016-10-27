#include "DaeValidator.h"
#include "COLLADASWConstants.h"
#include "PathUtil.h"
#include <iostream>
#include <set>

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__LINUX__)
#include <unistd.h>
#endif

using namespace COLLADASW;
using namespace std;

namespace opencollada
{
	class IdLine
	{
	public:
		IdLine(const string & id, size_t line)
			: mId(id)
			, mLine(line)
		{}

		bool operator < (const IdLine & other) const
		{
			return mId < other.mId;
		}

		const string & getId() const
		{
			return mId;
		}

		size_t getLine() const
		{
			return mLine;
		}

	private:
		string mId;
		size_t mLine = string::npos;
	};
}

namespace std
{
	template<>
	struct less<opencollada::IdLine>
	{
		bool operator () (const opencollada::IdLine& a, const opencollada::IdLine& b) const
		{
			return a < b;
		}
	};
}

namespace opencollada
{
	const char* colladaNamespace141 = "http://www.collada.org/2005/11/COLLADASchema";
	const char* colladaSchema141 = "collada_schema_1_4_1.xsd";

	const char* colladaNamespace15 = "http://www.collada.org/2008/03/COLLADASchema";
	const char* colladaSchema15 = "collada_schema_1_5.xsd";

	DaeValidator::DaeValidator(const Dae & dae)
		: mDae(dae)
	{}

	int DaeValidator::checkAll() const
	{
		int result = 0;
		result |= checkSchema();
		result |= checkUniqueIds();
		return result;
	}

	// Split string by whitespace
	vector<string> Split(const string & s)
	{
		vector<string> parts;
		istringstream iss(s);
		while (iss && !iss.eof())
		{
			string sub;
			iss >> sub;
			parts.emplace_back(sub);
		}
		return parts;
	}

	string GetExecutablePath()
	{
#if defined(_WIN32)
		vector<string::value_type> path;
		path.resize(MAX_PATH);
		GetModuleFileNameA(NULL, &path.front(), static_cast<DWORD>(path.size()));
		while (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			path.resize(path.size() * 2);
			GetModuleFileNameA(NULL, &path.front(), static_cast<DWORD>(path.size()));
		}
		return string(&path.front());
#elif defined(__APPLE__)
		// TODO test
		uint32_t size = 0;
		_NSGetExecutablePath(nullptr, &size);
		string path;
		path.resize(size - 1);
		if (_NSGetExecutablePath(&path.front(), &size) == 0)
			return path;
		return string();
#elif defined(__LINUX__)
		// TODO test
		static const char* proc_self_exe = "/proc/self/exe";
		stat st;
		if (lstat(proc_self_exe, &st) == 0)
		{
			string path;
			path.resize(st.st_size);
			if (readlink(proc_self_exe, &path.front(), st.st_size) != -1)
				return path;
		}
		return string();
#else
#error not implemented
#endif
	}

	string GetExecutableDirectory()
	{
		string exePath = GetExecutablePath();
		if (exePath.length() > 0)
		{
			size_t sep = exePath.rfind(Path::Separator());
			if (sep != string::npos)
			{
				return exePath.substr(0, sep);
			}
		}
		return string();
	}

	int DaeValidator::checkSchema(const string & schema_uri) const
	{
		int result = 0;
		if (schema_uri.empty())
		{
			// Get root <COLLADA> element
			auto collada = mDae.root();
			if (!collada)
			{
				cerr << "Can't find document root" << endl;
				return 1;
			}

			if (collada.name() != "COLLADA")
			{
				cerr << "Root element is not <COLLADA>" << endl;
				return 1;
			}

			// Get COLLADA namespace
			auto xmlns = collada.ns();
			if (!xmlns)
			{
				cerr << "COLLADA element has no namespace" << endl;
				return 1;
			}

			// Determine COLLADA version used by input dae file
			auto href = xmlns.href();
			if (href == colladaNamespace141)
			{
				result |= validateAgainstFile(Path::Join(GetExecutableDirectory(), colladaSchema141));
			}
			else if (href == colladaNamespace15)
			{
				result |= validateAgainstFile(Path::Join(GetExecutableDirectory(), colladaSchema15));
			}
			else
			{
				cerr << "Can't determine COLLADA version used by input file" << endl;
				return 1;
			}

			set<string> xsdURLs;

			// Find xsi:schemaLocation attributes in dae and try to validate against specified xsd documents
			auto elements = mDae.root().selectNodes("//*[@xsi:schemaLocation]");
			for (const auto & element : elements)
			{
				if (auto schemaLocation = element.attribute("schemaLocation"))
				{
					vector<string> parts = Split(schemaLocation.value());
					// Parse pairs of namespace/xsd and take second element
					for (size_t i = 1; i < parts.size(); i += 2)
					{
						xsdURLs.insert(parts[i]);
					}
				}
			}

			for (const auto & URL : xsdURLs)
			{
				int tmpResult = validateAgainstFile(URL);
				if (tmpResult == 2)
				{
					std::cout
						<< "Warning: can't load \"" << URL << "\"." << endl
						<< "Some parts of the document will not be validated." << endl;
				}
				else
				{
					result |= tmpResult;
				}
			}
		}
		else
		{
			// Validate against specified schema only
			result |= validateAgainstFile(schema_uri);
		}

		return result;
	}

	ostream & operator << (ostream & o, const COLLADABU::URI & uri)
	{
		o << uri.getURIString();
		return o;
	}

	int DaeValidator::checkUniqueIds() const
	{
		int result = 0;
		XmlNodeSet nodes = mDae.root().selectNodes("//*[@id]");
		set<IdLine> ids;
		for (const auto & node : nodes)
		{
			IdLine id_line(
				node.attribute(CSWC::CSW_ATTRIBUTE_ID).value(),
				node.line()
			);
			auto it = ids.find(id_line);
			if (it != ids.end())
			{
				cerr << mDae.getURI() << ":" << node.line() << ": Duplicated id \"" << id_line.getId() << "\". See first declaration at line " << it->getLine() << "." << endl;
				result |= 1;
			}
			else
			{
				ids.insert(id_line);
			}
		}
		return result;
	}

	int DaeValidator::validateAgainstFile(const string & xsdPath) const
	{
		// Open xsd
		cout << "Validating against " << xsdPath << endl;
		XmlSchema xsd;
		xsd.readFile(xsdPath.c_str());
		if (!xsd)
		{
			cerr << "Error loading " << xsdPath << endl;
			return 2;
		}

		// Validate dae against xsd
		if (!xsd.validate(mDae))
		{
			return 1;
		}

		return 0;
	}
}