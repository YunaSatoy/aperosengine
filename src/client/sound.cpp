/*
Minetest
Copyright (C) 2023 DS

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "sound.h"

#include <algorithm>
#include <string>
#include <vector>
#include <random>

#include "filesys.h"
#include "log.h"
#include "porting.h"
#include "settings.h"
#include "util/numeric.h"

std::vector<std::string> SoundFallbackPathProvider::getLocalFallbackPathsForSoundname(const std::string &name) {
	std::vector<std::string> paths;

	// Only try each name once
	if (m_done_names.count(name))
		return paths;
	m_done_names.insert(name);

	addThePaths(name, paths);

	// Remove duplicates
	std::sort(paths.begin(), paths.end());
	auto end = std::unique(paths.begin(), paths.end());
	paths.erase(end, paths.end());

	return paths;
}

void SoundFallbackPathProvider::addAllAlternatives(const std::string &common, std::vector<std::string> &paths) {
	static constexpr std::array<const char *, 11> extensions = {
		".ogg", ".0.ogg", ".1.ogg", ".2.ogg", ".3.ogg", ".4.ogg",
		".5.ogg", ".6.ogg", ".7.ogg", ".8.ogg", ".9.ogg"
	};
	paths.reserve(paths.size() + extensions.size());

	for (const auto &ext : extensions) {
		paths.emplace_back(common + ext);
	}
}

void SoundFallbackPathProvider::addThePaths(const std::string &name, std::vector<std::string> &paths) {
	addAllAlternatives(porting::path_share + DIR_DELIM + "sounds" + DIR_DELIM + name, paths);
	addAllAlternatives(porting::path_user + DIR_DELIM + "sounds" + DIR_DELIM + name, paths);
}

void ISoundManager::reportRemovedSound(sound_handle_t id) {
	if (id <= 0)
		return;

	freeId(id);
	m_removed_sounds.push_back(id);
}

sound_handle_t ISoundManager::allocateId(u32 num_owners) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<sound_handle_t> dis(1, SOUND_HANDLE_T_MAX - 1);

	while (m_occupied_ids.count(m_next_id) || m_next_id == SOUND_HANDLE_T_MAX) {
		m_next_id = dis(gen);
	}

	sound_handle_t id = m_next_id++;
	m_occupied_ids.emplace(id, num_owners);
	return id;
}

void ISoundManager::freeId(sound_handle_t id, u32 num_owners) {
	auto it = m_occupied_ids.find(id);
	if (it == m_occupied_ids.end())
		return;
	if (it->second <= num_owners) {
		m_occupied_ids.erase(it);
	} else {
		it->second -= num_owners;
	}
}

void sound_volume_control(ISoundManager *sound_mgr, bool is_window_active) {
	bool mute_sound = g_settings->getBool("mute_sound");
	if (mute_sound) {
		sound_mgr->setListenerGain(0.0f);
	} else {
		float old_volume = g_settings->getFloat("sound_volume");
		float new_volume = std::clamp(old_volume, 0.0f, 1.0f);

		if (old_volume != new_volume) {
			g_settings->setFloat("sound_volume", new_volume);
		}

		if (!is_window_active) {
			new_volume *= g_settings->getFloat("sound_volume_unfocused");
			new_volume = std::clamp(new_volume, 0.0f, 1.0f);
		}

		sound_mgr->setListenerGain(new_volume);
	}
}
