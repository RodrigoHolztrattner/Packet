////////////////////////////////////////////////////////////////////////////////
// Filename: PacketBackupManager.cpp
////////////////////////////////////////////////////////////////////////////////
#include "PacketBackupManager.h"
#include "../PacketFileHeader.h"
#include "../PacketFile.h"

///////////////
// NAMESPACE //
///////////////
PacketUsingDevelopmentNamespace(Packet)

PacketBackupManager::PacketBackupManager(std::filesystem::path _packet_path, std::filesystem::path _backup_path) :
    m_PacketPath(_packet_path),
    m_BackupPath(_backup_path)
{
	// Set the initial data
    m_IsActive = std::filesystem::exists(_backup_path) ? true : false;
}

PacketBackupManager::~PacketBackupManager()
{
}

bool PacketBackupManager::BackupFile(Path _file_path) const
{
    if (!m_IsActive)
    {
        return true;
    }

    // Determine the filesystem path to this file
    auto file_system_path = MergeSystemPathWithFilePath(m_PacketPath, _file_path);

    // Check if the file exist and is a valid file
    if (!std::filesystem::exists(file_system_path) || std::filesystem::is_directory(file_system_path))
    {
        return false;
    }

    // Determine the backup path
    auto backup_file_system_path = MergeSystemPathWithFilePath(m_BackupPath, _file_path);

    // Create the directory if it doesn't already exist
    if (!std::filesystem::create_directories(backup_file_system_path.parent_path()))
    {
        return false;
    }

    // Make the backup
    std::filesystem::copy(file_system_path, backup_file_system_path, std::filesystem::copy_options::overwrite_existing);

    return true;
}

bool PacketBackupManager::RestoreFile(Path _file_path) const
{
    if (!m_IsActive)
    {
        return true;
    }

    // Determine the filesystem path to this file and its backup path
    auto file_system_path = MergeSystemPathWithFilePath(m_PacketPath, _file_path);
    auto backup_file_system_path = MergeSystemPathWithFilePath(m_BackupPath, _file_path);

    // Check if the file exist
    if (!std::filesystem::exists(backup_file_system_path) || std::filesystem::is_directory(backup_file_system_path))
    {
        return false;
    }

    // Create the directory if it doesn't already exist (could happen if we delete a folder and we must
    // restore all deleted files)
    if (!std::filesystem::create_directories(file_system_path.parent_path()))
    {
        return false;
    }

    // Restore the backup
    std::filesystem::copy(backup_file_system_path, file_system_path, std::filesystem::copy_options::overwrite_existing);

    return true;
}