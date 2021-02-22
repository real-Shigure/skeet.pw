#include "animation_system.h"
#include "..\ragebot\aim.h"

void resolver::initialize(player_t* e, adjust_data* record, const float& goal_feet_yaw, const float& pitch)
{
	player = e;
	player_record = record;

	original_goal_feet_yaw = math::normalize_yaw(goal_feet_yaw);
	original_pitch = math::normalize_pitch(pitch);
}

void resolver::reset()
{
	player = nullptr;
	player_record = nullptr;

	was_first_bruteforce = false;
	was_second_bruteforce = false;

	original_goal_feet_yaw = 0.0f;
	original_pitch = 0.0f;
}

void resolver::resolve_yaw()
{
	player_info_t player_info;
	auto animstate = player->get_animation_state();

	if (!m_engine()->GetPlayerInfo(player->EntIndex(), &player_info) || !animstate || player_info.fakeplayer || g_cfg.player_list.disable_resolver[player_record->i] || !g_ctx.local()->is_alive() || player->m_iTeamNum() == g_ctx.local()->m_iTeamNum() || abs(TIME_TO_TICKS(player->m_flSimulationTime() - player->m_flOldSimulationTime()) - 1) || player_record->shot)
	{
		player_record->side = RESOLVER_ORIGINAL;
		return;
	}

	if (g_ctx.globals.missed_shots[player->EntIndex()])
	{
		switch (last_side)
		{
		case RESOLVER_ORIGINAL:
			g_ctx.globals.missed_shots[player->EntIndex()] = 0;
			break;
		case RESOLVER_ZERO:
			player_record->side = RESOLVER_LOW_POSITIVE;

			was_first_bruteforce = false;
			was_second_bruteforce = false;
			return;
		case RESOLVER_POSITIVE:
			player_record->side = was_second_bruteforce ? RESOLVER_ZERO : RESOLVER_NEGATIVE;

			was_first_bruteforce = true;
			return;
		case RESOLVER_NEGATIVE:
			player_record->side = was_first_bruteforce ? RESOLVER_ZERO : RESOLVER_POSITIVE;

			was_second_bruteforce = true;
			return;
		case RESOLVER_LOW_POSITIVE:
			player_record->side = RESOLVER_LOW_NEGATIVE;
			return;
		case RESOLVER_LOW_NEGATIVE:
			player_record->side = RESOLVER_POSITIVE;
			return;
		}
	}

	auto valid_move = true;
	if (animstate->m_velocity > 0.1f)
	{
		valid_move = animstate->m_flTimeSinceStartedMoving < 0.22f;
	}

	if (valid_move && player_record->layers[3].m_flWeight == 0.f && player_record->layers[3].m_flCycle == 0.f && player_record->layers[6].m_flWeight == 0.f)
	{
		auto delta = math::angle_difference(player->m_angEyeAngles().y, zero_goal_feet_yaw);
		auto positive_resolver = (2 * (delta <= 0.0f) - 1) > 0;
		player_record->side = positive_resolver ? RESOLVER_POSITIVE : RESOLVER_NEGATIVE;
	}
	else if (!valid_move &&	!(static_cast<int>(player_record->layers[12].m_flWeight * 1000.f)) && static_cast<int>(player_record->layers[6].m_flWeight * 1000.f) == static_cast<int>(previous_layers[6].m_flWeight * 1000.f))
	{
		auto delta_negative = abs(player_record->layers[6].m_flPlaybackRate - resolver_layers[0][6].m_flPlaybackRate);
		auto delta_zero = abs(player_record->layers[6].m_flPlaybackRate - resolver_layers[1][6].m_flPlaybackRate);
		auto delta_positive = abs(player_record->layers[6].m_flPlaybackRate - resolver_layers[2][6].m_flPlaybackRate);

		if (delta_zero < delta_positive || delta_negative <= delta_positive || (delta_positive * 1000.f))
		{
			if (delta_zero >= delta_negative && delta_positive > delta_negative && !(delta_negative * 1000.f))
			{
				player_record->side = RESOLVER_POSITIVE;
			}
		}
		else
		{
			player_record->side = RESOLVER_NEGATIVE;
		}
	}
	else
	{
		if (m_globals()->m_curtime - lock_side > 2.0f)
		{
			auto fire_data_positive = autowall::get().wall_penetration(g_ctx.globals.eye_pos, player->hitbox_position_matrix(HITBOX_HEAD, player_record->matrixes_data.positive), player);
			auto fire_data_negative = autowall::get().wall_penetration(g_ctx.globals.eye_pos, player->hitbox_position_matrix(HITBOX_HEAD, player_record->matrixes_data.negative), player);

			if (fire_data_positive.visible != fire_data_negative.visible)
			{
				player_record->side = fire_data_negative.visible ? RESOLVER_POSITIVE : RESOLVER_NEGATIVE;
				lock_side = m_globals()->m_curtime;
			}
			else
			{
				if (fire_data_positive.damage != fire_data_negative.damage)
				{
					player_record->side = fire_data_negative.damage > fire_data_positive.damage ? RESOLVER_POSITIVE : RESOLVER_NEGATIVE;
				}
			}
		}
	}
}

float resolver::resolve_pitch()
{
	return original_pitch;
}