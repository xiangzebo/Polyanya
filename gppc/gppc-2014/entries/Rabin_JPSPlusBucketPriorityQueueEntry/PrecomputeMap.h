/*
 * PrecomputeMap.h
 *
 * Copyright (c) 2015, Steve Rabin
 * All rights reserved.
 *
 * An explanation of the JPS+ algorithm and this code is contained in Chapter 14
 * of the book Game AI Pro 2, edited by Steve Rabin, CRC Press, 2015.
 * A copy of this code is on the website http://www.gameaipro.com.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * The name of the author may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY STEVE RABIN ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */ 

#pragma once
#include <vector>

enum ArrayDirections
{
	Down		= 0,
	DownRight	= 1,
	Right		= 2,
	UpRight		= 3,
	Up			= 4,
	UpLeft		= 5,
	Left		= 6,
	DownLeft	= 7,
	All			= 8
};

struct DistantJumpPoints
{
	short jumpDistance[8];
};

class PrecomputeMap
{
public:
	PrecomputeMap(int width, int height, std::vector<bool> map);
	~PrecomputeMap();

	DistantJumpPoints** CalculateMap();
	void SaveMap(const char *filename);
	DistantJumpPoints** LoadMap(const char *filename);
	void ReleaseMap() { if (m_mapCreated) DestroyArray(m_distantJumpPointMap); }

protected:
	bool m_mapCreated;
	int m_width;
	int m_height;
	std::vector<bool> m_map;
	unsigned char** m_jumpPointMap;
	DistantJumpPoints** m_distantJumpPointMap;

	template <typename T> void InitArray(T**& t, int width, int height);
	template <typename T> void DestroyArray(T**& t);

	void CalculateJumpPointMap();
	void CalculateDistantJumpPointMap();
	bool IsJumpPoint(int r, int c, int rowDir, int colDir);
	bool IsEmpty(int r, int c);
	bool IsWall(int r, int c);

	enum BitfieldDirections
	{
		MovingDown = 1 << 0,
		MovingRight = 1 << 1,
		MovingUp = 1 << 2,
		MovingLeft = 1 << 3,
	};
};

