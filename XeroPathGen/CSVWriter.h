//
// Copyright 2022 Jack W. Griffin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissionsand
// limitations under the License.
//
#pragma once
#include "ICsv.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

class CSVWriter
{
public:
	CSVWriter() = delete;
	~CSVWriter() = delete;

	template<class InputIt>
	static bool write(std::ostream& strm, const QVector<QString> headers, InputIt first, InputIt last)
	{
		for (int i = 0; i < headers.size(); i++)
		{
			strm << '"' << headers[i].toStdString() << '"';
			if (i != headers.size() - 1)
				strm << ",";
		}
		strm << std::endl;

		for (auto it = first; it != last; it++)
		{
			const ICsv& cl = *it;
			for (int i = 0; i < headers.size(); i++)
			{
				double v = cl.getField(headers[i]);
				if (i != 0)
					strm << ",";

				strm << v;
			}
			strm << std::endl;
		}

		return true;
	}
};


