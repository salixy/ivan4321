# Helper script to invite collaborator
$token = $env:GITHUB_TOKEN
if (-not $token) {
    Write-Host "Please set GITHUB_TOKEN environment variable."
    exit 1
}
$headers = @{
    "Authorization" = "token $token"
    "User-Agent"    = "Antigravity"
    "Accept"        = "application/vnd.github.v3+json"
}

try {
    $res = Invoke-RestMethod -Uri "https://api.github.com/repos/salixy/ivan4321/collaborators/blessfields" -Method Put -Headers $headers
    Write-Host "Invitation sent successfully!"
    $res | ConvertTo-Json
} catch {
    Write-Host "Error sending invitation:"
    Write-Host $_.Exception.Message
}
